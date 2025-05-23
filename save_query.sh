#!/bin/bash

# Use current date if no argument is provided
if [ -z "$1" ]; then
  DATE=$(date +%F)  # Format: YYYY-MM-DD
  echo "No date provided. Using current date: $DATE"
else
  DATE="$1"
fi

# Cost estimate
echo -e "\nCost per run:"
echo "Estimated ~17.39 GB scanned on BigQuery"
echo "Cost per TB scanned = \$5"
echo "17.39 GB * (1 TB / 1000 GB) * \$5 = ~\$0.08695\n"

# Variables
TABLE_NAME=$(date -d "$DATE" "+%m_%d_%y")
FOLDER_NAME=$(date -d "$DATE" "+%m-%d-%y")
DATASET="gencast_export_data"
PROJECT="ultra-task-456813-d5"
BUCKET="gencast-export-bucket"

echo "→ Running query and saving to: $TABLE_NAME..."

bq query \
  --nouse_legacy_sql \
  --destination_table="$PROJECT:$DATASET.$TABLE_NAME" \
  --replace \
  --use_cache=false \
  "
  SELECT
    f.time AS forecast_time,
    ST_Y(t.geography) AS latitude,
    ST_X(t.geography) AS longitude,
    e.2m_temperature AS temp_2m
  FROM
    \`$PROJECT.weathernext_gen_forecasts.126478713_1_0\` AS t,
    UNNEST(t.forecast) AS f,
    UNNEST(f.ensemble) AS e
  WHERE
    t.init_time = TIMESTAMP(\"$DATE\")
    AND e.ensemble_member = '5'
  ORDER BY forecast_time, latitude, longitude
  "

echo "→ Exporting table to GCS: gs://$BUCKET/$FOLDER_NAME/"
bq extract \
  --destination_format=CSV \
  "$PROJECT:$DATASET.$TABLE_NAME" \
  "gs://$BUCKET/$FOLDER_NAME/$TABLE_NAME-*.csv"

echo "→ Deleting BigQuery table: $DATASET.$TABLE_NAME"
bq rm -f -t "$PROJECT:$DATASET.$TABLE_NAME"

echo "→ Creating local directory: $FOLDER_NAME"
mkdir -p "$FOLDER_NAME"

echo "→ Downloading from GCS to ./$FOLDER_NAME/"
gsutil -m cp "gs://$BUCKET/$FOLDER_NAME/*" "$FOLDER_NAME/"

echo "→ Deleting GCS folder: gs://$BUCKET/$FOLDER_NAME/"
gsutil -m rm -r "gs://$BUCKET/$FOLDER_NAME/"

echo "Download complete, processing into population-weighted CSV."

./merge
