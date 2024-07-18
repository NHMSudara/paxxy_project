import pandas as pd

# Load the CSV file
file_path = 'ADS_BHI_Data.csv'
data = pd.read_csv(file_path, header=None)

# Create the new column with alternating 1s and 0s
new_column = [1 if i % 2 == 0 else 0 for i in range(len(data))]

# Insert the new column at position 9 (index 8)
data.insert(8, 'NewColumn', new_column)

# Save the updated dataframe back to a CSV file
updated_file_path = 'Updated_ADS_BHI_Data.csv'
data.to_csv(updated_file_path, header=False, index=False)

print(f"Updated CSV saved to {updated_file_path}")
