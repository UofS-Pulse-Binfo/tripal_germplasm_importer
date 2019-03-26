Features
========
Germplasm Cross Importer is developed following `Tripal Developer’s Guide » Creating Custom Data Loaders <https://tripal.readthedocs.io/en/latest/dev_guide/custom_data_loader.html>`_.

File Upload
-----------
Requirements for upload files can be found easily in the module.

.. image:: features.1.file_upload.png

Prefix and Organism
-------------------
Organism must be selected from dropdown menu before upload.
Default uniquename for each germplam is 'GERM' followed by it's `stock id <https://laceysanderson.github.io/chado-docs/stock/tables/stock.html>`_ and user can give a unique prefix to replace the default value.

.. image:: features.2.prefix_organism.png


Bulk load germplasm crosses
---------------------------
As Chado is the data store for Tripal, germplasm data will be saved in five `chado tables <https://laceysanderson.github.io/chado-docs/index.html>`_: cv, cvterm, stock, stockprop, and stock_relationship in this module.

  - specific control vocabularies (cv) and cvterms are required for this module

  - each germplasm cross will be loaded into table stock

  - properties for each germplasm will be loaded into table stockprop

  - relationships with parents for each germplasm will be loaded into table stock_relationship


  .. note::

    CV and cvterms used in this module is customized for our database, please configure array CV and array CVTERM based on your database.

PHP UnitTest
------------
PHP UnitTest is created to guarantee information of each germplasm can be inserted into the right places of database.
