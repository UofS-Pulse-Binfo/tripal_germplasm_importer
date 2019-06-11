<?php

/**
 * @see TripalImporter
 */
 class GermplasmAccessionImporter extends TripalImporter {

  /**
   * The name of this loader.  This name will be presented to the site
   * user.
   */
  public static $name = 'Germplasm Accession Importer';

  /**
   * The machine name for this loader. This name will be used to construct
   * the URL for the loader.
   */
  public static $machine_name = 'tripal_germplasm_accession_importer';

  /**
   * A brief description for this loader.  This description will be
   * presented to the site user.
   */
  public static $description = 'Imports breeding accessions into Chado.';

  /**
   * An array containing the extensions of allowed file types.
   */
  public static $file_types = ['tsv', 'txt'];

  public static $use_analysis = FALSE;
  /**
   * If the $use_analysis value is set above then this value indicates if the
   * analysis should be required.
   */
  public static $require_analysis = FALSE;

  /**
   * Provides information to the user about the file upload.  Typically this
   * may include a description of the file types allowed.
   */
  public static $upload_description = 'Germplasm file should be a tab separated file with the following colums:<ol>
      <li>Accession Number: A unique identifier for the accession. </li>
      <li>Germplasm Genus: The genus of the accession.</li>
      <li>Germplasm Species: The species of the accession.</li>
      <li>Germplasm Name: Name of this germplasm accession.</li>
      <li>Germplasm PUI: The Permanent Unique Identifier which represents a germplasm.</li>
      <li>External Database</li>
      <li>Institute Code: The code for the Institute that has bred the material.</li>
      <li>Institute Name: The name of the Institute that has bred the material.</li>
      <li>Pedigree: The cross name and optional selection history.</li>
      <li>Country of Origin Code</li>
      <li>Synonym: The synonyms of the accession.</li>
      <li>Default Display Name: Human readable name used for display purposes.</li>
      <li>Seed Source: The source of the seed.</li>
      <li>Acquisition Date: The date this germplasm was aquired by the genebank.</li>
      <li>Biological Status of Accession Code: The 3 digit code representing the biological status of the accession.</li>
      <li>Breeding Method DbId: The unique identifier for the breeding method used to create this germplasm.</li>
      <li>subtaxa: Subtaxon can be used to store any additional taxonomic identifier.</li>
      <li>Type of Germplasm Storage Code: The 2 digit code representing the type of storage this germplasm is kept in at a genebank. </li>
    </ol>';

  /**
   * Indicates the methods that the file uploader will support.
   */
  public static $methods = array(
    // Allow the user to upload a file to the server.
    'file_upload' => TRUE,
    // Allow the user to provide the path on the Tripal server for the file.
    'file_local' => TRUE,
    // Allow the user to provide a remote URL for the file.
    'file_remote' => TRUE,
  );

  public function form($form, &$form_state){
    $sql = "SELECT DISTINCT genus FROM {organism}";
    $org_rset = chado_query($sql);
    $genus = array();
    $genus[''] = '';
    while ($one_genus = $org_rset->fetchObject()) {
      $genus[$one_genus->genus] = $one_genus->genus;
      //array_push($genus, $one_genus->genus);
    }

    $form['instructions'] = [
      '#weight' => -100,
      '#markup' => '
        <h2>Load germplasm into database</h2>
        <p>Upload file must in TSV (tab-separated values) format. Please confirm file format and column order before upload.</p>
      ',
    ];

    $form['genus_name'] = [
      '#type' => 'select',
      '#title' => t('Genus'),
      '#options' => $genus,
      '#required' => TRUE,
      '#description' => t('Select the genus for germplasm accession file you would like to upload'),
    ];
    /* @TO-DO update available species for this genus
    $form['available_species'] = [
      '#type' => 'textarea',
      '#title' => t('Available Species under this Genus in Database'),
      '#description' => t('All available species under selected genus, please make sure all species in file can match to one species.'),
    ];
    // display species in database
    if(isset($form_state['values']['genus_name'])){
      $sql = "SELECT DISTINCT species FROM {organism} WHERE genus = :genus_selected";
      $args = array(':genus_selected' => $form_state['values']['genus_name']);
      $org_rset = chado_query($sql, $args);
      $species_inDB = array();
      while ($one_species = $org_rset->fetchObject()) {
        $species_inDB[$one_species->species] = $one_species->species;
      }
      $form['available_species']['default_value'] = $species_inDB;
    }
    */
    dpm($form);
    return $form;
  }


  /**
   * @see TripalImporter::formValidate()
   */
  public function formValidate($form, &$form_state){
    //parent::formValidate($form, $form_state);
    $genus_name = $form_state['values']['genus_name'];
    dpm($form_state, 'form state');
    if (!$genus_name){
      form_set_error('genus_name','Please choose a genus');
    }
  }

  /**
  * @see TripalImporter::run()
  */
  public function run(){
    $arguments = $this->arguments['run_args'];
    $file_path = $this->arguments['files'][0]['file_path'];

    $analysis_id = $arguments['analysis_id'];
    $genus_name = $arguments['genus_name'];
    $this->loadGermplasmAccession($file_path, $genus_name, $dbxref_id = NULL, $description = NULL, $is_obsolete = f);
  }

  public function loadGermplasmAccession($file_path, $genus, $user_prefix, $dbxref_id, $description, $is_obsolete){
    // print upload file name in Job LOGS

    print("genus name: $genus/n");

    $file_name_exp = explode('/', $file_path);
    $this->logMessage('Upload file name: !input_file ',['!input_file' => array_pop($file_name_exp)], TRIPAL_WARNING);

    $CV = array(
      'schema' => array(
        'name' => 'schema',
        'definition' => 'Schema.org. Schema.org is sponsored by Google, Microsoft, Yahoo and Yandex. The vocabularies are developed by an open community process.',
        'need_insert_by_function' => '1'
      ),
      'ncit' => array(
        'name' => 'ncit',
        'definition' => 'The NCIt OBO Edition project aims to increase integration of the NCIt with OBO Library ontologies. NCIt is a reference terminology that includes broad coverage of the cancer domain, including cancer related diseases, findings and abnormalities. NCIt OBO Edition releases should be considered experimental.',
        'need_insert_by_function' => '1'
      ),
      'MCPD' => array(
        'name' => 'multicrop passport ontology',
        'definition' => '',
        'need_insert_by_function' => '0'
      ),
      'GCP' => array(
        'name' => 'GCP germplasm ontology',
        'definition' => '',
        'need_insert_by_function' => '0'
      )
    );
    //read cvterm
    $CVTERM = array(
      'accession_number' => array(
        'name' => 'accession number',
        'cv_name' => 'MCPD',
        'need_insert_by_function' => '0'
      ),
      'germplasm_genus' => array(
        'name' => 'genus',
        'cv_name' => 'MCPD',
        'need_insert_by_function' => '0'
      ),
      'germplasm_species' => array(
        'name' => 'species',
        'cv_name' => 'MCPD',
        'need_insert_by_function' => '0'
      ),
      'germplasm_name' => array(
        'name' => 'accession name',
        'cv_name' => 'GCP',
        'need_insert_by_function' => '0'
      ),
      'germplasm_PUI' => array(
        'name' => 'url',
        'is_obsolete' => 0,
        'is_relationshiptype' => 0,
        'definition' => 'URL of the item.',
        'cv_name' => 'schema',
        'accession' => 'url',
        'db_name' => 'schema',
        'need_insert_by_function' => '1'
      ),
      'institute_code' => array(
        'name' => 'institute code',
        'cv_name' => 'MCPD',
        'need_insert_by_function' => '0'
      ),
      'institute_name' => array(
        'name' => 'Breeding institute name',
        'cv_name' => 'MCPD',
        'need_insert_by_function' => '0'
      ),
      'pedigree' => array(
        'name' => 'pedigree information',
        'cv_name' => 'GCP',
        'need_insert_by_function' => '0'
      ),
      'country_of_origin_code' => array(
        'name' => 'country of origin',
        'cv_name' => 'GCP',
        'need_insert_by_function' => '0'
      ),
      'synonym' => array(
        'name' => 'synonym',
        'cv_name' => 'GCP',
        'need_insert_by_function' => '0'
      ),
      'default_display_name' => array(
        'name' => 'common crop name',
        'cv_name' => 'MCPD',
        'need_insert_by_function' => '0'
      ),
      'seed_source' => array(
        'name' => 'source',
        'is_obsolete' => 0,
        'is_relationshiptype' => 0,
        'definition' => '',
        'cv_name' => 'ncit',
        'accession' => '',
        'db_name' => 'NCIT',
        'need_insert_by_function' => '1'
      ),
      'acquisition_date' => array(
        'name' => 'acquisition date',
        'cv_name' => 'MCPD',
        'need_insert_by_function' => '0'
      ),
      'biological_status_of_accession_code' => array(
        'name' => 'biological status of accession',
        'cv_name' => 'GCP',
        'need_insert_by_function' => '0'
      ),
      'breeding_method_DbId' => array(
        'name' => 'method',
        'is_obsolete' => 0,
        'is_relationshiptype' => 0,
        'definition' => '',
        'cv_name' => 'ncit',
        'accession' => '',
        'db_name' => 'NCIT',
        'need_insert_by_function' => '1'
      ),
      'subtaxa' => array(
        'name' => 'subtaxa',
        'cv_name' => 'MCPD',
        'need_insert_by_function' => '0'
      ),
      'type_of_germplasm_storage_code' => array(
        'name' => 'type of germplasm storage',
        'cv_name' => 'GCP',
        'need_insert_by_function' => '0'
      )
    );

    //database various so we need names for both cvterm and cv to determine specific cvterm id for a cvterm
    $CV = $this->check_and_insert_cv($CV);
    //@test print_r($CV);
    $CVTERM = $this->check_and_insert_cvterm($CVTERM, $CV);
    //@test print_r($CVTERM);
    //From DOC (https://tripal.readthedocs.io/en/latest/dev_guide/custom_data_loader.html)
    // We want to provide a progress report to the end-user so that they:
    // 1) Recognize that the loader is not hung if running a large file, but is
    //    executing
    // 2) Provides some indicatation for how long the file will take to load.
    //
    // Here we'll get the size of the file and tell the TripalImporter how
    // many "items" we have to process (in this case bytes of the file).
    $filesize = filesize($file_path);
    $this->setTotalItems($filesize);
    $this->setItemsHandled(0);
    $bytes_read = 0;

    if(!file_exists($file_path)){
       var_dump(['Could not find file: ', $file_path]);
    }

    $fh = fopen($file_path, 'r');

    //read file, and explode each line
    while ($line_germplasm = fgets($fh)){

      //From DOC (https://tripal.readthedocs.io/en/latest/dev_guide/custom_data_loader.html)
      // Calculate how many bytes we have read from the file and let the
      // importer know how many have been processed so it can provide a
      // progress indicator.
      $bytes_read += drupal_strlen($line_germplasm);
      $this->setItemsHandled($bytes_read);

      $line_germplasm = trim($line_germplasm);
      $germ_col = explode("\t", $line_germplasm);

      //TO-DO: header check, confirm column order
      //should be in pre-determined order
      if (preg_match('/^Accession/', $line_germplasm) ){
        continue;
      }

      /**For each germplasm line
      * since genus should be selected from drop-down box, so it exists in chado already
      * cvterm should be checked, selected/inserted and cvterm_id should be reported to array CVTERM
      * 1.  check for organism
      * 2.  check and insert this germplasm accession
      * 3.  check and insert info to table dexref, and db
      * 4.  check and insert properties to table stockprop
      * 5.  check and insert properties to table synonym, and stock_synonym
      */
      // skip comments
      if (preg_match('/^#/', $line_germplasm)) {
        continue;
      }

      // skip empty lines
      if (preg_match('/^\s*$/', $line_germplasm)) {
        continue;
      }

      $spreadsheet_format = array(
        '1' => 'accession_number',
        '2' => 'germplasm_genus',
        '3' => 'germplasm_species',
        '4' => 'germplasm_name',
        '5' => 'germplasm_PUI',
        '6' => 'external_database',
        '7' => 'institute_code',
        '8' => 'institute_name',
        '9' => 'pedigree',
        '10' => 'country_of_origin_code',
        '11' => 'synonym',
        '12' => 'default_display_name',
        '13' => 'seed_source',
        '14' => 'acquisition_date',
        '15' => 'biological_status_of_accession_code',
        '16' => 'breeding_method_DbId',
        '17' => 'subtaxa',
        '18' => 'type_of_germplasm_storage_code'
      );

      $one_accession = array();
      // save required values(c1 to c8) into array
      for ($i=1;$i<=4;$i++){
        $one_accession[$spreadsheet_format[$i]] = $germ_col[$i-1];
      }
      // save optional values(c9 to c18) into array
      for ($i=5;$i<=18;$i++){
        if ($germ_col[$i-1]){
          $one_accession[$spreadsheet_format[$i]] = $germ_col[$i-1];
        }
      }
      //@test
      print_r($one_accession);
      /*#################################################################
      * step 1: check chado talbes: organism, find matched organsim (organism_id)
      * get genus_name using genus(user selected) and (2)germplasm_species
      * record $organism_id
      */
      if ($genus != $one_accession['germplasm_genus']){
        $this->logMessage('WARNING:, !genus, in file is different from your choosen genus', ['!genus' => $one_accession['germplasm_genus']], TRIPAL_WARNING);
      }
      $match = array(
        'genus' => $genus,
        'species' => $one_accession['germplasm_species']
      );
      if ($one_accession['subtaxa']){
        $match['infraspecific_name'] = $one_accession['subtaxa'];
      }
      $results = chado_select_record('organism', ['organism_id'], $match);
      if (count($results) > 1){
          $this->logMessage('ERROR: The genus, !genus, species, !species, has more than one matched organism in chado.organism.',['!genus' => $genus, '!species' => $one_accession['germplasm_species']], TRIPAL_WARNING);
          break;
      }
      elseif (count($results) == 1){
        //@log $this->logMessage('The stock, !stock, has one matched name in chado.stock.',['!stock' =>  $one_accession['name']], TRIPAL_WARNING);
        $organism_id = $results[0]->organism_id;
      }
      else{
        $this->logMessage('ERROR: The genus, !genus, species, !species, has no matched organism in chado.organism.',['!genus' => $genus, '!species' => $one_accession['germplasm_species']], TRIPAL_WARNING);
      }

      //@test
      print("S1: the organism_id for this germplasm accession: $organism_id. \n\n");
      // check/insert this germ accession to table:stock

      /*#################################################################
      * step 2: load to table:stock
      * check this (c3)germplasm name not exist table:stock with $organism_id
      * report if yes, also record stock_id and dbxref_id
      */
      $match = array(
        'name' =>  $one_accession['germplasm_name'],
        'organism_id' => $organism_id,
      );
      $results = chado_select_record('stock', ['stock_id','uniquename'], $match);
      if (count($results) >= 2){
          $this->logMessage('ERROR: The stock, !stock, has more than one matched names in chado.stock.',['!stock' =>  $one_accession['name']], TRIPAL_WARNING);
          break;
      }
      elseif (count($results) == 1){
        //@log $this->logMessage('The stock, !stock, has one matched name in chado.stock.',['!stock' =>  $one_accession['name']], TRIPAL_WARNING);
        $germplasm_stock_id = $results[0]->stock_id;
        if ($results[0]->uniquename){
          if($results[0]->uniquename != $one_accession['accession_number']){
            //@TO-DO: if uniquename exists already but not match to the accession_number we want to update, FOR THE MOMENT, we won't make any change/update, but it should be!!!
            $this->logMessage('WARNING: The uniquename/accession of exist, !uniquename, in chado.stock does not match loading file accession number, !accession, IMPORTANT!',['!uniquename'=>$results[0]->uniquename, '!accession'=>$one_accession['accession_number']], TRIPAL_WARNING);
          }
          else{
            $germplasm_uniquename_accession = $results[0]->uniquename;
          }
        }
        else{
          $value_matched = array(
            'stock_id' => $germplasm_stock_id
          );
          $value_update = array(
            'uniquename' => $one_accession['accession_number']
          );
          $results = chado_update_record('stock', $value_matched, $value_update);
          if ($results == FALSE){
            $this->logMessage('WARNING: The uniquename update for table:stock:unique of germplasm accession, !stock failed.',['!stock' => $one_accession['germplasm_name']], TRIPAL_WARNING);
          }
          else{
            $germplasm_uniquename_accession = $results[0]->uniquename;
          }
        }
      }
      else{
        // then try to insert this germplasm into table:stock
        //@log $this->logMessage('The stock, !stock, has no matched name in chado.stock, start insertion.',['!stock' =>  $one_accession['name']], TRIPAL_WARNING);
        // uniquename of each germplasm is a random string prefiexed with Germplasm: for now, need to update it latter
        // load as cvterm: accesssion
        $value = array(
          'organism_id' => $organism_id,
          'name' =>  $one_accession['germplasm_name'],
          'uniquename' => $one_accession['accession_number'],
          'type_id' => $CVTERM['germplasm_name']['type_id'],
          'dbxref_id' => $dbxref_id,
          'description' => $description,
          'is_obsolete' => $is_obsolete
        );
        $results = chado_insert_record('stock', $value);
        //@test
        print("Insertion results: \n");
        print_r($results);
        if (count($results) == 0){
          $this->logMessage('The import of germplasm to table:stock, !stock failed.',['!stock' =>  $one_accession['name']], TRIPAL_WARNING);
        }
        else{
          // save stock_id for next step
          $germplasm_stock_id = $results['stock_id'];
          $germplasm_uniquename_accession = $results['uniquename'];
        }
      }
      //@test
      print("S2: the stock_id for this germplasm accession: $germplasm_stock_id. \n");
      print("S2: the uniquename/accession for this germplasm accession: $germplasm_uniquename_accession. \n\n");

      /*#################################################################
      * step 3: load to chado:dbxref abd chado:db
      *   3.1: using (c1)accession_number step to check chado:dbxref.accession
      *     insert/update record to chado:dbxref.dbxref_id, chado:dbxref.accession=(c1)accession_number
      *   3.2: using (c6)external_database check chado:db.name
      *     insert/update record to chado:db.db_id,  chado:db.urlprefix=(c5)germplasm_pui
      *     update record chado:dbxref.db_id
      */

      if($germplasm_uniquename_accession){
        // 3.1
        $match = array(
          'accession' => $germplasm_uniquename_accession
        );
        $results_dbxref = chado_select_record('dbxref', ['dbxref_id', 'db_id'], $match);

        if (count($results_dbxref) >= 2){
            $this->logMessage('ERROR: The dbxref.accession, !dbxref, has more than one matched accession in chado:dbxref.',['!dbxref' =>   $germplasm_uniquename_accession], TRIPAL_WARNING);
            break;
        }
        elseif (count($results_dbxref) == 1){
          $germplasm_dbxref_id = $results_dbxref[0]->dbxref_id;
        }
        else{
          // insert this record to chado:dbxref
          $value = array(
            'accession' => $germplasm_uniquename_accession,
            'db_id' => 1,
            'version' => 0
          );
          $results = chado_insert_record('dbxref', $value);

          if ($results == FALSE){
            $this->logMessage('The dbxref insertion for table:dbxref of germplasm accession, !stock failed.',['!stock' => $one_accession['germplasm_name']], TRIPAL_WARNING);
          }
          else{
            $germplasm_dbxref_id = $results['dbxref_id'];
          }
        }
        //@test
        print("S3.1: the dbxref_id for this germplasm accession: $germplasm_dbxref_id. \n\n");

        // update dbxref_id back to chado:stock.dbxref_id
        $value_matched = array(
          'stock_id' => $germplasm_stock_id
        );
        $value_update = array(
          'dbxref_id' => $germplasm_dbxref_id
        );
        chado_update_record('stock', $value_matched, $value_update);

        //3.2
        if (($one_accession['external_database']) && ($one_accession['germplasm_PUI'])){
          $match = array(
            'name' => $one_accession['external_database']
          );
          $results_db = chado_select_record('db', ['db_id','urlprefix'], $match);

          if (count($results_db) >= 2){
              $this->logMessage('ERROR: The db, !db, has more than one matches in chado:db.',['!db' =>  $one_accession['external_database']], TRIPAL_WARNING);
              break;
          }
          elseif (count($results_db) == 1){

            // check/update chado:db.urlprefix
            $db_urlprefix = $results_db[0]->urlprefix;
            if( $db_urlprefix ){
              if ($one_accession['germplasm_PUI'] != $db_urlprefix){
                $this->logMessage('WARNING: The db_urlprefix, !db_urlprefix, exists in chado.dbxref but not match to loading file germplasm_PUI: !file_url.',['!db_urlprefix' =>  $db_urlprefix, '!file_url'=> $one_accession['germplasm_PUI']], TRIPAL_WARNING);
              }
            }
            else{
              $value_matched = array(
                'db_id'=> $results_db[0]->db_id
              );
              $value_update = array(
                'urlprefix' => $one_accession['germplasm_PUI']
              );
              $results = chado_update_record('db', $value_matched, $value_update);
              if ($results == FALSE){
                $this->logMessage('WARNING: The accession_number update for table:dbxref:accession of stock, !stock failed.',['!stock' => $one_accession['germplasm_name']], TRIPAL_WARNING);
              }
            }

            // at this point, dbxref_id for current germplasm accession must exist in chado.dbxref, check/update chado:dbxref.db_id
            if ($results_dbxref->db_id){
              if ($results_db->db_id != $results_dbxref->db_id){
                $this->logMessage('WARNING: The db_id, !db_id, exists in chado.dbxref but not match to loading file external_database id: !file_db_id.',['!db_id' =>  $results_dbxref->db_id, '!file_db_id'=> $results_db->db_id], TRIPAL_WARNING);
              }
            }
            else{
              $value_matched = array(
                'dbxref_id'=> $germplasm_dbxref_id
              );
              $value_update = array(
                'db_id' => $results_db->db_id
              );
              $results = chado_update_record('dbxref', $value_matched, $value_update);
              if ($results == FALSE){
                $this->logMessage('WARNING: The db_id update for table:dbxref:db_id of germplasm accession, !stock failed.',['!stock' => $one_accession['germplasm_name']], TRIPAL_WARNING);
              }
            }

          }
          else{
            $this->logMessage('WARNING: The db, !db, has no match in chado:db.',['!db' =>  $one_accession['external_database']], TRIPAL_WARNING);
          }
        }
        else{
          //@test
          print("S3.2 skipped due becaue culumns External Database/Germplasm PUI not exist. \n");
        }
      }
      else{
        //@test
        print("S3 skipped because uniquename/accession of existed and new doesn't match. \n");
      }
      /*#################################################################
      * step 4: load to table:stockprop
      *   check/insert properties of this germ accession to table:stockprop using stcok_id from step2
      *   function load_stockprop($stockprop_name, $stockprop_stock_id, $stockprop_type_id, $stockprop_value)
      *   ??? (c9)pedigree
      */
      $column_insert_2_stockprop = array(
        'institute_code',
        'institute_name',
        'pedigree',
        'country_of_origin_code',
        'seed_source',
        'acquisition_date',
        'biological_status_of_accession_code',
        'breeding_method_DbId',
        'type_of_germplasm_storage_code'
        );

      foreach($column_insert_2_stockprop as $one_prop){
        if ($one_accession[$one_prop]){
          print("Germplasm name:". $one_accession['germplasm_name']. " stock_id: $germplasm_stock_id  type_id: ". $CVTERM[$one_prop]['type_id']. " property value:" . $one_accession[$one_prop]. " \n\n");
          $this -> load_stockprop($one_accession['germplasm_name'], $germplasm_stock_id, $CVTERM[$one_prop]['type_id'], $one_accession[$one_prop]);
        }
      }
      //@test
      print("S4 finished. \n\n");
      /*#################################################################
      * step 5: load other values
      *  5.1 (c11)synonyms (require new chado table stock_synonym)
      *     a.load synonoyms to chado:synonym
      *     b.also, load synonym-stock relationship to chado:stock_synonym
      *  5.2 ??? (c12)default_display_name
      *
      */
      if ($one_accession['synonym']){
        $all_synonyms = explode(';', $one_accession['synonym']);
        foreach($all_synonyms as $one_synonym){
          // 5.1.a
          $one_synonym = trim($one_synonym);
          $match = array(
            'name' => $one_synonym,
            'type_id' => $CVTERM['synonym']['type_id']
          );
          $results_synonym = chado_select_record('synonym', ['synonym_id'], $match);
          if (count($results_synonym) >= 2){
            $this->logMessage('ERROR: The synonym, !synonym, has more than one matches in chado:synonym.',['!synonym' => $one_synonym], TRIPAL_WARNING);
          }
          elseif(count($results_synonym) == 1){
            $one_synonym_id = $results_synonym[0]->synonym_id;
          }
          else{
            $value = array(
              'name' => $one_synonym,
              'type_id'=> $CVTERM['synonym']['type_id'],
              'synonym_sgml'=> ''
            );
            $results = chado_insert_record('synonym',$value);

            if (count($results) == 0){
              $this->logMessage('The import of synonym, !one_synonym, to table:synonym for germplasm accession, !stock, failed.',['!stock' =>  $one_accession['germplasm_name'], '!one_synonym' => $one_synonym], TRIPAL_WARNING);
            }
            else{
              $one_synonym_id = $results['synonym_id'];
            }
          }
          //5.1.b
          $match = array(
              'synonym_id' => $one_synonym_id
          );
          $results_stockSynonym = chado_select_record('stock_synonym', ['stock_synonym_id'], $match);
          if (count($results_stockSynonym) >= 2){
             $this->logMessage('ERROR: The synonym, !synonym, has more than one matches in chado:stock_synonym.',['!synonym' => $one_synonym], TRIPAL_WARNING);
          }
          elseif(count($results_stockSynonym) == 1){

          }
          else{
            $value = array(
              'synonym_id' => $one_synonym_id,
              'stock_id'=> $germplasm_stock_id,
              'pub_id'=> 1,
              'is_current' => NULL,
              'is_internal' => NULL
            );
            $results = chado_insert_record('stock_synonym',$value);
            if (count($results) == 0){
              $this->logMessage('The import of synonym, !one_synonym, to table:stock_synonym for germplasm accession, !stock, failed.',['!stock' =>  $one_accession['germplasm_name'], '!one_synonym' => $one_synonym], TRIPAL_WARNING);
            }
          }
        } //end of foreach synonym
      }

    } //end reading each line

  } //end of function loadGermplasmAccession


    /** check all cv required in this module
     *  after insertion/checking, update type_id(cvterm_id), cv_id, dbxref_id to array $CV
     *  @param $CV
     */
    public function check_and_insert_cv($CV){
      foreach($CV as $key_hr_name =>$one_cv){
         $result_cv = chado_select_record('cv', ['cv_id'], ['name'=>$one_cv['name'] ]);
         if (count($result_cv) == 0) {
           if ($one_cv['need_insert_by_function'] == 1){
             $result_cv = chado_insert_cv($one_cv['name'], $one_cv['definition']);
             $CV[$key_hr_name]['cv_id'] = $result_cv['cv_id'];
           }
           else{
             $this->logMessage('ERROR: The cv, !cv, has no match in chado.cvterm which should be inserted in installation of this module.',['!cv' => $one_cv['name']], TRIPAL_WARNING);
             break;
           }
         }
         else{
           $CV[$key_hr_name]['cv_id'] = $result_cv[0]->cv_id;
         }
      }
      return $CV;
    }


     /** check all cvterms required in this module
     * Three situatioins for each:
     *    1. only one match, use it, and save cvterm_id for further use as type_id
     *    2. no match, insert this term using api chado_insert_cvterm()
     *    3. more than one match, then use cv name
     *  after insertion/checking, update type_id(cvterm_id), cv_id, dbxref_id to array $CVTERM
     *  @param $CVTERM
     */
     public function check_and_insert_cvterm($CVTERM, $CV){
       //check each cvterm
       foreach($CVTERM as $key_hr_name =>$one_cvterm){
         $result_cvterm = chado_select_record('cvterm', ['cvterm_id','cv_id', 'dbxref_id'], ['name'=>$one_cvterm['name'], 'cv_id'=>$CV[$one_cvterm['cv_name']]['cv_id']]);
         if (count($result_cvterm) == 0 ) {
           if ($one_cvterm['need_insert_by_function'] == 1){
             $term = array(
               'name' => $one_cvterm['name'],
               'is_obsolete' => $one_cvterm['is_obsolete'],
               'is_relationshiptype' => $one_cvterm['is_relationshiptype'],
               'definition' => $one_cvterm['definition'],
               'cv_name' => $one_cvterm['cv_name'],
               'id' => $one_cvterm['db_name'].':'.$one_cvterm['accession']
             );
             chado_insert_cvterm($term);
             $result_cvterm = chado_select_record('cvterm', ['cvterm_id','cv_id', 'dbxref_id'], ['name'=>$one_cvterm['name'], 'cv_id'=>$CV[$one_cvterm['cv_name']]['cv_id']]);
           }
           else{
             $this->logMessage('ERROR: The cvterm, !cvterm, has no match in chado.cvterm which should be inserted in installation of this module.',['!cvterm' => $one_cvterm['name']], TRIPAL_WARNING);
             break;
           }
         }
         elseif(count($result_cvterm) == 1){
           //@log $this->logMessage('The cvterm, !cvterm, has one match in chado.cvterm',['!cvterm' => $one_cvterm['name']], TRIPAL_WARNING);
         }
         else{
          $this->logMessage('WARNING: The cvterm, !cvterm, has more than one match in chado.cvterm and no cvterm_id supplied for a unique match.',['!cvterm' => $one_cvterm['name']], TRIPAL_WARNING);
         }
         $CVTERM[$key_hr_name]['type_id'] = $result_cvterm[0]->cvterm_id;
         $CVTERM[$key_hr_name]['cv_id'] = $CV[$one_cvterm['cv_name']]['cv_id'];
         $CVTERM[$key_hr_name]['dbxref_id'] = $result_cvterm[0]->dbxref_id;
       }
       return $CVTERM;
     }

   /** a function used to load extra info of a germplasm to table:stockprop
   *  all cvterm should haved already checked out
   *  1. check if this stock property has alreay been inserted
   *  2. insert if not
   *  3. report error is more than matches are found
   *
   *  @param  $stockprop_name
   *  @param  $stockprop_stock_id
   *  @param  $stockprop_type_id
   *  @param  $stockprop_value
   */
   public function load_stockprop($stockprop_name, $stockprop_stock_id, $stockprop_type_id, $stockprop_value){

     $match = array(
       'stock_id' => $stockprop_stock_id,
       'type_id' => $stockprop_type_id
     );
     $result_stockprop = chado_select_record('stockprop', ['stockprop_id'], $match);

     if (count($result_stockprop) == 0){
       $values = array(
         'stock_id' => $stockprop_stock_id,
         'type_id' => $stockprop_type_id,
         'value' => $stockprop_value
       );
       $results = chado_insert_record('stockprop', $values);
     }
     elseif(count($result_stockprop) == 1){
       //@log $this->logMessage('Stock property, !stockprop, exists in table:stockprop already.', ['!stockprop' => $stockprop_name], TRIPAL_WARNING);
     }
     else{
       // report for error, this property for this stock should only have one match
       $this->logMessage('Stock property, !stockprop, has multiple matches in table:stockprop.', ['!stockprop' => $stockprop_name], TRIPAL_WARNING);
     }
   }


} //end of this class
