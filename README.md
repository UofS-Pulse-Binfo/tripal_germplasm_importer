# Tripal Germplasm Importer

This module will load germplasm data from file with a drush command. Data will be saved in Chado.

This module is developed following [Tripal Developer's Guide: Creating Custom Data Loaders](https://tripal.readthedocs.io/en/latest/dev_guide/custom_data_loader.html).

## Dependencies
1. Tripal 3
2. PostgreSQL

## Germplasm File Format
This module supports loading of spreadsheet file formats (txt, tst, csv, tsv):
```
Year 	Season	Cross No.	Maternal Parent	Paternal Parent	Cross Type	Seed Type	Cotyledon Colour	Seed Coat	Comment	Male Type	Female Type
2000	Winter	ABC	ABC_M	ABC_P	Backcross	Yellow					
