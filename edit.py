from netCDF4 import Dataset
import pandas as pd
import numpy as np

# Load dataset
ds = Dataset("demographics_hybrid_1950_2020_15_min.nc")

# Variables
lat = ds.variables['latitude'][:]   # 720
lon = ds.variables['longitude'][:]  # 1440
year = ds.variables['year'][:]      # 71 years
pop = ds.variables['demographic_totals']  # shape (720, 1440, 14, 71)

# Find index of year 2020
year_index = np.where(year == 2020)[0][0]

# Pull total population for 2020, demographic type index 0
pop_2020 = pop[:, :, 0, year_index]  # shape (720, 1440)

# Make grid for lat/lon
lat_grid, lon_grid = np.meshgrid(lat, lon, indexing='ij')  # shape (720, 1440)

# Sanity check
assert pop_2020.shape == lat_grid.shape == lon_grid.shape

# Flatten and save
df = pd.DataFrame({
    "latitude": lat_grid.ravel(),
    "longitude": lon_grid.ravel(),
    "population": (pop_2020.ravel() * 1_000_000).astype(int)
})

# Filter
df = df.dropna()
df = df[df["population"] > 0]

# Output
df.to_csv("population_2020.csv", index=False)
print("✅ Done. Saved as population_2020.csv")

