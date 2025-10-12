#!/bin/bash

set -eu

# Load config
if [ -r "/etc/alpsnap.conf" ]; then
  . /etc/alpsnap.conf
elif [ -r "./etc/alpsnap.conf" ]; then
  . ./etc/alpsnap.conf
else
  echo "No config found, copy etc/alpsnap.conf to /etc"
  exit 1
fi

log() {
  local ts; ts="$(date '+%Y-%m-%d %H:%M:%S')"
  echo "$ts [alpsnap] $*" | tee -a "$LOG_FILE"
}

die() { log "ERROR: $*"; exit 1; }

require_root() { [ "$(id -u)" -eq 0 ] || die "Run as root"; }

snapshot_id() { date +"$TIME_FMT"; }

ensure_dirs() { mkdir -p "$SNAP_ROOT"; }

check_deps() {
  command -v rsync >/dev/null 2>&1 || die "Missing dependency: rsync"
  command -v tar   >/dev/null 2>&1 || die "Missing dependency: tar"

  if [ "$COMPRESS_MODE" = "zstd" ]; then
    command -v zstd >/dev/null 2>&1 || die "Missing dependency: zstd (apt install zstd)"
  fi

  if [ "$ENCRYPT_MODE" = "gpg" ]; then
    command -v gpg >/dev/null 2>&1 || die "Missing dependency: gpg (apt install gnupg)"
  fi
}

# tmpdir and cleanup
tmpdir=""
cleanup() {
  if [ -n "${tmpdir:-}" ] && [ -d "$tmpdir" ]; then
    rm -rf "$tmpdir"
  fi
}
trap cleanup EXIT

# ---------------- Snapshot creation ----------------
create_snapshot() {
  require_root
  check_deps
  ensure_dirs

  local sid; sid="$(snapshot_id)"
  tmpdir="$(mktemp -d)"

  log "Snapshot begin id=$sid"

  for p in $INCLUDE_PATHS; do
    local rel; rel="$(echo "$p" | sed 's#^/##')"
    [ -z "$rel" ] && rel="root"
    local tgt="$tmpdir/$rel"
    mkdir -p "$tgt"
    log "rsync snapshot of $p -> $tgt"
    rsync -aHAX --delete --numeric-ids \
      $(for e in $EXCLUDE_PATHS; do echo "--exclude=$e"; done) \
      "$p/" "$tgt/" || die "rsync failed for $p"
  done

  # compress
  local archive="$SNAP_ROOT/$sid.tar"
  case "$COMPRESS_MODE" in
    zstd) archive="$SNAP_ROOT/$sid.tar.zst"; tar -I zstd -cf "$archive" -C "$tmpdir" . ;;
    tar)  archive="$SNAP_ROOT/$sid.tar.gz";  tar -czf "$archive" -C "$tmpdir" . ;;
    none) archive="$SNAP_ROOT/$sid.tar";     tar -cf "$archive" -C "$tmpdir" . ;;
  esac

  # encrypt
  if [ "$ENCRYPT_MODE" = "gpg" ]; then
    if [ -n "$GPG_RECIPIENT" ]; then
      gpg --output "$archive.gpg" --encrypt --recipient "$GPG_RECIPIENT" "$archive"
    else
      gpg --symmetric --output "$archive.gpg" "$archive"
    fi
    rm -f "$archive"
    archive="$archive.gpg"
  fi

  log "Snapshot archived: $archive"
}

# ---------------- Restore ----------------
restore_snapshot() {
  require_root
  local sid="$1"
  local archive

  archive="$(ls "$SNAP_ROOT/$sid".tar* 2>/dev/null | head -n1 || true)"
  [ -n "$archive" ] || die "Snapshot archive not found: $sid"

  tmpdir="$(mktemp -d)"

  # decrypt
  if echo "$archive" | grep -q '\.gpg$'; then
    log "Decrypting $archive"
    gpg --no-symkey-cache --output "$tmpdir/snapshot.tar" --decrypt "$archive" || die "Decrypt failed"
    archive="$tmpdir/snapshot.tar"
  fi

  # extract
  log "Extracting $archive"
  case "$archive" in
    *.tar.zst) tar -I zstd -xf "$archive" -C "$tmpdir" ;;
    *.tar.gz)  tar -xzf "$archive" -C "$tmpdir" ;;
    *.tar)     tar -xf "$archive" -C "$tmpdir" ;;
    *) die "Unknown archive format: $archive" ;;
  esac

  # rsync
  for p in $INCLUDE_PATHS; do
    local rel; rel="$(echo "$p" | sed 's#^/##')"
    [ -z "$rel" ] && rel="root"
    local s="$tmpdir/$rel"
    [ -d "$s" ] || { log "WARN: path missing in snapshot: $rel"; continue; }
    log "Restoring $p from $sid"
    rsync -aHAX --delete --numeric-ids "$s/" "$p" || die "Restore failed for $p"
  done

  log "Restore complete for $sid"
}

# ---------------- List ----------------
list_snapshots() {
  ls -1 "$SNAP_ROOT" 2>/dev/null \
    | grep -E '\.tar(\.gz|\.zst)?(\.gpg)?$' \
    | sed -E 's/\.tar(\.gz|\.zst)?(\.gpg)?$//' \
    | sort -u || true
  return 0
}

usage() {
  cat <<EOF
Usage: ./alpsnap.sh <command> [args]

Commands:
  snapshot             Create a snapshot (archive)
  restore <id>         Restore snapshot <id>
  list                 List snapshots
  help                 Show this help
EOF
}

main() {
  local cmd="${1:-help}"
  case "$cmd" in
    snapshot) create_snapshot ;;
    restore)
      shift
      [ $# -ge 1 ] || die "restore requires snapshot id"
      restore_snapshot "$1"
      ;;
    list) list_snapshots ;;
    help|*) usage ;;
  esac
}

main "$@"
