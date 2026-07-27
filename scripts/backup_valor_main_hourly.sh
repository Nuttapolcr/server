#!/usr/bin/env bash
set -euo pipefail

project_dir='/home/valor_ro/server-test'
conf_file="$project_dir/conf/inter_athena.conf"
backup_dir="$project_dir/backups/hourly"

exec 9>"/tmp/valor_main_hourly_backup.lock"
flock -n 9 || exit 0

get_conf() {
	awk -F: -v key="$1" '
		$1 ~ "^[[:space:]]*" key "[[:space:]]*$" {
			sub(/^[^:]*:[[:space:]]*/, "")
			sub(/[[:space:]]*\/\/.*$/, "")
			sub(/[[:space:]]*#.*/, "")
			gsub(/^[[:space:]]+|[[:space:]]+$/, "")
			print
			exit
		}
	' "$conf_file"
}

db_host="$(get_conf login_server_ip)"
db_port="$(get_conf login_server_port)"
db_user="$(get_conf login_server_id)"
db_pass="$(get_conf login_server_pw)"
db_name="$(get_conf login_server_db)"

if [[ "$db_name" != 'valor_main' ]]; then
	printf 'Refusing to back up unexpected database: %s\n' "$db_name" >&2
	exit 1
fi

mkdir -p "$backup_dir"
timestamp="$(date +%Y%m%d_%H%M%S)"
final_file="$backup_dir/valor_main_hourly_${timestamp}.sql.gz"
partial_file="$final_file.partial"

MYSQL_PWD="$db_pass" mysqldump \
	--host="$db_host" \
	--port="$db_port" \
	--user="$db_user" \
	--single-transaction \
	--quick \
	--routines \
	--triggers \
	--events \
	--hex-blob \
	--default-character-set=utf8mb4 \
	--databases "$db_name" | gzip -9 > "$partial_file"

gzip -t "$partial_file"
mv "$partial_file" "$final_file"

# Keep completed hourly backups from today and the previous calendar day only.
today="$(date +%Y%m%d)"
yesterday="$(date --date='yesterday' +%Y%m%d)"
find "$backup_dir" -maxdepth 1 -type f \
	-name 'valor_main_hourly_*.sql.gz' \
	! -name "valor_main_hourly_${today}_*.sql.gz" \
	! -name "valor_main_hourly_${yesterday}_*.sql.gz" \
	-delete

printf 'Created %s\n' "$final_file"
