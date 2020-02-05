Germplasm Cross Importer
========================

Prepare a Germplasm Cross File
------------------------------
Germplasm Cross Importer allows bulk load germplasm crosses into a database.
Germplasm cross file needs to follow a specific templet to be able to upload. The following columns must include:

1.	**Year**: the year the cross was made in
2.	**Season**: the season the cross was made in

  2.1 Make sure to have the full name of a season

3.	**Cross Number**: a unique identifier for the cross (e.g. 6673S)

  3.1 Cross number may already exist in database, double check to make sure the cross number matches exactly the stocked name in database

4.	**Maternal Parent**: the name of the maternal parent of this cross

  4.1	Name of the maternal parent may already exist in database, double check to make sure the cross number matches exactly the stocked name in database

5.	**Paternal Parent**: the name of the paternal parent of this cross

  5.1	Name of the paternal parent may already exist in database, double check to make sure the cross number matches exactly the stocked name in database

6.	**Cross Type**: the type of cross (e.g. Single Cross, Back Cross)

  6.1	Cross type information may be able to find from Cross number. A capitalized letter tends to appear within a cross number, which indicates the Cross type. “S” stands for single cross, “M” stands for multiple cross, “T” stands for triple cross, and “B” stands for back cross. The letter may also be found in low case or missing.

7.	**Seed Type**: either the market class or seed coat colour of the progeny
8.	**Cotyledon Colour**: the cotyledon colour of the seed resulting from the cross
9.	**Comment**: a free-text comment about the cross

Add more columns as needed (e.g. Seed coat, Male Cotyledon Color, Female Cotyledon Color).


Prefix and Organism
-------------------
Organism must be selected from dropdown menu before upload.
Prefix text box is optional to fill in and default the value is 'GERM'.
The uniquename for each germplasm will be 'GERM' followed by it's `stock id <https://laceysanderson.github.io/chado-docs/stock/tables/stock.html>`_ but user can give a unique prefix to replace 'GERM'.

.. image:: cross.2.prefix_organism.png


Bulk load germplasm crosses
---------------------------
As Chado is the data store for Tripal, germplasm corsses will be saved in five `chado tables <https://laceysanderson.github.io/chado-docs/index.html>`_: cv, cvterm, stock, stockprop, and stock_relationship in this module.

  - required control vocabularies (CVs) and CV terms will be checked before data loading

  - germplasm crosses will be loaded into table stock

  - properties for each germplasm will be loaded into table stockprop

  - relationships with parents for each germplasm will be loaded into table stock_relationship
