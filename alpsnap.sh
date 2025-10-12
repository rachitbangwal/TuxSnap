#!/bin/ash

set -eu

# Load config
if [ -r "/etc/alpsnap.conf" ]; then
  . /etc/alpsnap.conf
elif [ -r "./etc/alpsnap.conf" ]; then
  . ./etc/alpsnap.conf
else
  echo "No config found, Manually copy etc/alpsnap.conf to /etc"
  exit 1
fi

check_deps() {
  for dep in rsync; do
    command -v "$dep" >/dev/null 2>&1 || {
      echo "Missing dependency: $dep. Install with: apk add $dep"
      exit 1
    }
  done
}

log() {
  local ts; ts="$(date '+%Y-%m-%d %H:%M:%S')"
  echo "$ts [alpsnap] $*" | tee -a "$LOG_FILE"
}

die() { log "ERROR: $*"; exit 1; }

require_root() { [ "$(id -u)" -eq 0 ] || die "Run as root"; }

cmd_exists() { command -v "$1" >/dev/null 2>&1; }

snapshot_id() { date +"$TIME_FMT"; }

detect_backend() {
  if [ "$SNAP_BACKEND" = "auto" ]; then
    if cmd_exists btrfs && awk '$2=="/"{print $3}' /proc/mounts | grep -q '^btrfs$'; then
      echo "btrfs"; return
    fi
    if cmd_exists lvdisplay; then
      echo "lvm"; return
    fi
    echo "rsync"; return
  fi
  echo "$SNAP_BACKEND"
}

ensure_dirs() { mkdir -p "$SNAP_ROOT"; }

rsync_snapshot() {
  local sid="$1" prev="$2"
  local dest="$SNAP_ROOT/$sid"
  mkdir -p "$dest"

  for p in $INCLUDE_PATHS; do
    local rel; rel="$(echo "$p" | sed 's#^/##')"
    [ -z "$rel" ] && rel="root"

    local tgt="$dest/$rel"
    mkdir -p "$tgt"

    local linkopt=""
    if [ -n "$prev" ] && [ -d "$SNAP_ROOT/$prev/$rel" ]; then
      linkopt="--link-dest=$SNAP_ROOT/$prev/$rel"
    fi

    log "rsync snapshot of $p -> $tgt${linkopt:+ (link-dest=$SNAP_ROOT/$prev/$rel)}"

    rsync -aHAX --delete --numeric-ids $linkopt \
      $(for e in $EXCLUDE_PATHS; do echo "--exclude=$e"; done) \
      "$p/" "$tgt/" || die "rsync failed for $p"
  done

  log "rsync snapshot completed: $sid"
}

create_snapshot() {
  require_root
  check_deps
  ensure_dirs

  local backend; backend="$(detect_backend)"
  local sid; sid="$(snapshot_id)"

  local prev; prev="$(ls -1 "$SNAP_ROOT" 2>/dev/null | sort | tail -n1 || true)"

  log "Snapshot begin id=$sid backend=$backend prev=${prev:-none}"

  case "$backend" in
    rsync) rsync_snapshot "$sid" "$prev" ;;
    btrfs) rsync_snapshot "$sid" "$prev" ;;
    lvm)   rsync_snapshot "$sid" "$prev" ;;
    *)     die "Unsupported backend filesystem: $backend" ;;
  esac

  log "Snapshot done id=$sid"
}

restore_snapshot() {
  require_root
  local sid="$1"
  local src="$SNAP_ROOT/$sid"
  [ -d "$src" ] || die "Snapshot not found: $sid"

  for p in $INCLUDE_PATHS; do
    local rel; rel="$(echo "$p" | sed 's#^/##')"
    [ -z "$rel" ] && rel="root"
    local s="$src/$rel"
    [ -d "$s" ] || { log "WARN: path missing in snapshot: $rel"; continue; }
    log "Restoring $p from $sid"
    rsync -aHAX --delete --numeric-ids "$s/" "$p/" || die "Restore failed for $p"
  done
  log "Restore complete for $sid"
}



list_snapshots() { ls -1 "$SNAP_ROOT" 2>/dev/null | sort || true; }

usage() {
  cat <<EOF
Usage: ./alpsnap.sh <command> [args]

Commands:
  snapshot             Create a snapshot
  restore <id>         Restore snapshot <id>
  list                 List snapshots
  help                 Show this help
EOF
}

main() {
  local cmd="${1:-help}"
  case "$cmd" in
    snapshot) create_snapshot ;;
    restore)  [ $# -ge 2 ] || die "restore requires snapshot id"; restore_snapshot "$2" ;;
    list)     list_snapshots ;;
    help|*)   usage ;;
  esac
}

main "$@"
