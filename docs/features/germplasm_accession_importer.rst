Germplasm Accession Importer
============================

File Upload
-----------
Format requirements for upload files can be found easily in UI while using this module.

.. image:: accession.1.file_format.png

Genus
-----
Genus of the accessions in file must be selected from dropdown menu before upload. All accessions in one file must belong to same genus and match this selection.

.. image:: accession.2.genus.png


Bulk load germplasm accessions
------------------------------
As Chado is the data store for Tripal, germplasm accessions will be saved in several `chado tables <https://laceysanderson.github.io/chado-docs/index.html>`_: cv, cvterm, stock, stockprop, db, dbxref, synonym and stock_synonym for this importer.

The general idea of how accession information will be saved in database:

- required control vocabularies (CVs) and CV terms will be checked before data loading

- organism of one accession is determined by germplasm genus, species and subtaxa(optional)

- germplasm accession will be loaded into table stock, dexref, and db

- properties will be loaded into table stockprop

- synonyms will be loaded into table synonym and stock_synonym
