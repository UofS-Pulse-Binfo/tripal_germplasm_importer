<?php

/**
 * @see TripalImporter
 */
 class GermplasmCrossImporter extends TripalImporter {

  /**
   * The name of this loader.  This name will be presented to the site
   * user.
   */
  public static $name = 'Germplasm Cross Importer';

  /**
   * The machine name for this loader. This name will be used to construct
   * the URL for the loader.
   */
  public static $machine_name = 'tripal_germplasm_cross_importer';

  /**
   * A brief description for this loader.  This description will be
   * presented to the site user.
   */
  public static $description = 'Imports breeding crosses into Chado.';

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
      <li>Year: the year the cross was made in.</li>
      <li>Season: the season the cross was made in (e.g. Spring, Fall, Winter, Summer).</li>
      <li>Cross Number: a unique identifier for the cross (e.g. 1234S).</li>
      <li>Maternal Parent: the name of the maternal parent of this cross.</li>
      <li>Paternal Parent: the name of the paternal parent of this cross.</li>
      <li>Cross Type: the type of cross (e.g. single, double, triple).</li>
      <li>Seed Type: either the market class or seed coat colour of the seed resulting from the cross.</li>
      <li>Cotyledon Colour: the cotyledon colour of the seed resulting from the cross.</li>
      <li>Comment: a free-text comment about the cross.</li>
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
    $sql = "SELECT * FROM {organism} ORDER BY genus, species";
    $org_rset = chado_query($sql);
    $organisms = array();
    $organisms[''] = '';
    while ($organism = $org_rset->fetchObject()) {
      $organisms[$organism->organism_id] = "$organism->genus $organism->species ($organism->common_name)";
    }

    $form['instructions'] = [
      '#weight' => -100,
      '#markup' => '
        <h2>Load germplasm into database</h2>
        <p>Upload file must in TSV (tab-separated values) format. Please confirm file format and column order before upload.</p>
      ',
    ];

    $form['prefix'] = [
      '#type' => 'textfield',
      '#title' => 'Prefix',
      '#description' => 'Please give a prefix for this germplasm cross. Unique names for germplasm contained in file will be this prefix followed by stock id. Unique prefix can help you track this germplasm in the future, and default prefix is GERM.<br><b>NOTE:</b> unique names for existed germplasm cross will be updated with new prefix too.',
    ];

    $form['organism_id'] = [
      '#type' => 'select',
      '#title' => t('Organism'),
      '#options' => $organisms,
      '#required' => TRUE,
      '#description' => t('Select the organism for germplasm file you would like to upload'),
    ];

    return $form;
  }


  /**
   * @see TripalImporter::formValidate()
   */
  public function formValidate($form, &$form_state){
    //parent::formValidate($form, $form_state);
    $organism_id = $form_state['values']['organism_id'];

    if ($organism_id == 0){
      form_set_error('organism_id','Please choose an organism');
    }
  }

  /**
  * @see TripalImporter::run()
  */
  public function run(){
    $arguments = $this->arguments['run_args'];
    $file_path = $this->arguments['files'][0]['file_path'];

    $analysis_id = $arguments['analysis_id'];
    $organism_id = $arguments['organism_id'];
    $user_prefix = $arguments['prefix'];

    $this->loadGermplasmCross($file_path, $organism_id, $user_prefix, $dbxref_id = NULL, $description = NULL, $is_obsolete = f);
  }

  public function loadGermplasmCross($file_path, $organism, $user_prefix, $dbxref_id, $description, $is_obsolete){

    // print upload file name in Job LOGS
    $file_name_exp = explode('/', $file_path);
    $this->logMessage('Upload file name: !input_file ',['!input_file' => array_pop($file_name_exp)], TRIPAL_WARNING);

    $CV = array(
      'schema' => array(
        'name' => 'schema',
        'definition' => 'Schema.org. Schema.org is sponsored by Google, Microsoft, Yahoo and Yandex. The vocabularies are developed by an open community process.'
        ),
      'stock_relationship' => array(
        'name' => 'stock_relationship',
        'definition' => 'A local vocabulary that contains types of relationships between stocks.'
        ),
      'stock_property' => array(
        'name' => 'stock_property',
        'definition' => 'A local vocabulary that contains types of relationships between stocks.'
        ),
      'local' => array(
        'name' => 'local',
        'definition' => 'Locally created terms.'
        ),
      'stock_type' => array(
        'name' => 'stock_type',
        'definition' => 'A local vocabulary that contains a list of types for stocks.'
        ),
      );

    //read cvterm
    $CVTERM = array(
      'stock_type' => array(
        'name' => 'F1',
        'is_obsolete' => 0,
        'is_relationshiptype' => 0,
        'definition' => 'The first generation of offsprint from a parental cross.',
        'cv_name' => 'stock_type',
        'accession' => 'F1',
        'db_name' => 'tripal'
      ),
      'cross_type' => array(
        'name' => 'additionalType',
        'is_obsolete' => 0,
        'is_relationshiptype' => 0,
        'definition' => '',
        'cv_name' => 'schema',
        'accession' => 'additionalType',
        'db_name' => 'schema'
      ),
      'maternal_parent' => array(
        'name' => 'is_maternal_parent_of',
        'is_obsolete' => 0,
        'is_relationshiptype' => 0,
        'definition' => '',
        'cv_name' => 'stock_relationship',
        'accession' => 'is_maternal_parent_of',
        'db_name' => 'tripal'
      ),
      'paternal_parent' => array(
        'name' => 'is_paternal_parent_of',
        'is_obsolete' => 0,
        'is_relationshiptype' => 0,
        'definition' => '',
        'cv_name' => 'stock_relationship',
        'accession' => 'is_paternal_parent_of',
        'db_name' => 'tripal'
      ),
      'season' => array(
        'name' => 'crossingblock_season',
        'is_obsolete' => 0,
        'is_relationshiptype' => 0,
        'definition' => 'Indicates that the current stock was the progeny of a cross made in the season specified.',
        'cv_name' => 'stock_property',
        'accession' => 'crossingblock_season',
        'db_name' => 'tripal'
      ),
      'year' => array(
        'name' => 'crossingblock_year',
        'is_obsolete' => 0,
        'is_relationshiptype' => 0,
        'definition' => 'Indicates that the current stock was the progeny of a cross made in the year specified.',
        'cv_name' => 'stock_property',
        'accession' => 'crossingblock_year',
        'db_name' => 'tripal'
      ),
      'cotyledon_colour' => array(
        'name' => 'cotyledon_colour',
        'is_obsolete' => 0,
        'is_relationshiptype' => 0,
        'definition' => 'The colour of the cotyledon',
        'cv_name' => 'stock_property',
        'accession' => 'cotyledon_colour',
        'db_name' => 'tripal'
      ),
      'seed_type' => array(
        'name' => 'seed_type',
        'is_obsolete' => 0,
        'is_relationshiptype' => 0,
        'definition' => 'Characteristics or observable phenotypes of the seed.',
        'cv_name' => 'stock_property',
        'accession' => 'seed_type',
        'db_name' => 'tripal'
      ),
      'seed_coat_colour' => array(
        'name' => 'seed_coat_colour',
        'is_obsolete' => 0,
        'is_relationshiptype' => 0,
        'definition' => 'The colour of the seed coat',
        'cv_name' => 'stock_property',
        'accession' => 'seed_coat_colour',
        'db_name' => 'tripal'
      ),
      'comment' => array(
        'name' => 'comment',
        'is_obsolete' => 0,
        'is_relationshiptype' => 0,
        'definition' => 'Comments, typically from users. ',
        'cv_name' => 'schema',
        'accession' => 'comment',
        'db_name' => 'schema'
      )
    );


    //database various so we need names for both cvterm and cv to determine specific cvterm id for a cvterm
    $CV = $this->check_and_insert_cv($CV);
    $CVTERM = $this->check_and_insert_cvterm($CVTERM, $CV);

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

      //TO-DO: header check, confirm column order
      //should be in order:
      //1.Year 2.Season 3.Cross No. 4.Maternal Parent 5.paternal Parent 6.Cross Type
      //7.Seed Type 8.Cotyledon Colour 9.Seed Coat 10.Comment
      if (preg_match('/^Year/', $line_germplasm) ){
        continue;
      }

      /**For each germplasm line
      * since this organism is selected from drop-down box, so it exists in chado already
      * cvterm should be checked, selected/inserted and cvterm_id should be reported to array CVTERM
      * 1.  check and insert this germplasm
      * 2.  check and insert properties of this germplasm
      * 3.  check and insert relationships with parents
      */
      // skip comments
      if (preg_match('/^#/', $line_germplasm)) {
        continue;
      }

      // skip empty lines
      if (preg_match('/^\s*$/', $line_germplasm)) {
        continue;
      }

      $germ_col = explode("\t", $line_germplasm);

      $stock = array(
        'name' => trim($germ_col[2])
      );
      if($germ_col[0]){
        $stock['year'] = trim($germ_col[0]);
      }
      if($germ_col[1]){
        $stock['season'] = trim($germ_col[1]);
      }
      if($germ_col[3]){
        $stock['maternal_parent'] = trim($germ_col[3]);
      }
      if($germ_col[4]){
        $stock['paternal_parent'] = trim($germ_col[4]);
      }
      if($germ_col[5]){
        $stock['cross_type'] = trim($germ_col[5]);
      }
      if($germ_col[6]){
        $stock['seed_type'] = trim($germ_col[6]);
      }
      if($germ_col[7]){
        $stock['cotyledon_colour'] = trim($germ_col[7]);
      }
      if($germ_col[8]){
        $stock['seed_coat_colour'] = trim($germ_col[8]);
      }
      if($germ_col[9]){
        $stock['comment'] = trim($germ_col[9]);
      }

      //$this->logMessage(("\n\n".'Loading !stock.'),['!stock' => $stock['name']], TRIPAL_WARNING);
      //************************************************************************
      //FIRSRT: load to table Stock
      //check this germplasm name not exist in db
      // report error is yes
      $match = array(
        'name' => $stock['name'],
        'organism_id' => $organism
      );
      $results = chado_select_record('stock', ['stock_id','name'], $match);

      if (count($results) >= 2){
          $this->logMessage('ERROR: The stock, !stock, has more than one matched names in chado.stock.',['!stock' => $stock['name']], TRIPAL_WARNING);
          break;
      }
      elseif (count($results) == 1){
        //@log $this->logMessage('The stock, !stock, has one matched name in chado.stock.',['!stock' => $stock['name']], TRIPAL_WARNING);
        $germplasm_stock_id = $results[0]->stock_id;
      }
      else{
        // then try to insert this germplasm into table:stock
        //@log $this->logMessage('The stock, !stock, has no matched name in chado.stock, start insertion.',['!stock' => $stock['name']], TRIPAL_WARNING);
        // uniquename of each germplasm is a random string prefiexed with Germplasm: for now, need to update it latter

        $value = array(
          'organism_id' => $organism,
          'name' => $stock['name'],
          'uniquename' => uniqid("Germplasm:"),
          'type_id' => $CVTERM['stock_type']['type_id'],
          'dbxref_id' => $dbxref_id,
          'description' => $description,
          'is_obsolete' => $is_obsolete
        );
        chado_insert_record('stock', $value);

        $match = array(
          'name' => $stock['name'],
          'organism_id' => $organism
        );
        $results = chado_select_record('stock', ['stock_id'], $match);

        if (count($results) == 0){
          $this->logMessage('The import of germplasm to table:stock, !stock failed.',['!stock' => $stock['name']], TRIPAL_WARNING);
        }elseif (count($results) == 1){
          // save stock_id for next step
          $germplasm_stock_id = $results[0]->stock_id;
        }
      }

      // always update unique name, for both situations of this stock need insertion or exists already
      // check user input on prefix, use default if not exist
      if(!empty($user_prefix)){
        $unique_name_4update = $user_prefix.$germplasm_stock_id;
      }
      else{
        $unique_name_4update = 'GERM'.$germplasm_stock_id;
      }

      $update_match = array(
        'name' => $stock['name'],
        'organism_id' => $organism,
        'stock_id' => $germplasm_stock_id
        );
      $update_value = array(
        'uniquename' => $unique_name_4update
        );

      $results = chado_update_record('stock', $update_match, $update_value);

      if ($results == FALSE){
        $this->logMessage('The uniquename update for stock, !stock failed.',['!stock' => $stock['name']], TRIPAL_WARNING);
      }


      //  load all extra info to table:stockprop
      //  use array $CVTERM and array $stock to match (names from file header) / (variable name) / (database cvterm)
      //  loop though all keys in array $stock
      foreach($stock as $key=> $value){
        // check if this key exists in array $CVTERM
        if (($key == 'name') || ($key == 'maternal_parent') || ($key == 'paternal_parent')){
          continue;
        }

        if ($CVTERM[$key]){
          // check if this cvterm exist in table:cvterm
          // we should have cheked this cvterm, double check just for sure
          $match = array(
            'name' => $CVTERM[$key]['name']
          );
          $results = chado_select_record('cvterm', ['cvterm_id'], $match);
          if ($results == 0){
            //@log $this->logMessage('ERROR: Cvterm , !cvterm, does not exist in databas table:cvterm.', ['!cvterm' => $CVTERM[$key]['name']], TRIPAL_WARNING);
          }
          else{
            //  use function to check/load this property to talbe:stockprop
            $this->load_stockprop($key, $germplasm_stock_id, $CVTERM[$key]['type_id'], $value);
          }
        }
        else{
          //@log $this->logMessage('Column name , !header_name, does not exist in array CVTERM.', ['!header_name' => $key], TRIPAL_WARNING);
        }
      }

      //TO-DO: check if insertion is successful?

      //  check maternal parent exist in db
      if ($stock['maternal_parent']){
        $match = array(
          'name' => $stock['maternal_parent'],
          'organism_id' => $organism
        );
        $results = chado_select_record('stock', ['stock_id'], $match);
        $maternal_parent_stock_id = $results[0]->stock_id;

        if (count($results) == 0){
          // relationship with maternal parent should not exist, check it
          $result_sr_not_exist = chado_select_record('stock_relationship', ['stock_relationship_id'], ['object_id'  => $germplasm_stock_id, 'type_id' => $CVTERM['maternal_parent']['type_id'] ]);
          if (count($result_sr_not_exist) != 0){
            $this->logMessage('WARNING: Maternal parent for !stock_relationship NOT exist in DB but parent relationship does.', ['!stock_relationship' => $stock['name']], TRIPAL_WARNING);
          }
        }
        elseif (count($results) == 1){
          //  load maternal parent to table stock_relationship
          $this->load_stock_relationship($stock['name'], $maternal_parent_stock_id, $germplasm_stock_id, $CVTERM['maternal_parent']['type_id']);

          //  load 'female parent' as a property table:stockprop
          //$shit->load_stockprop($stock['name'], $germplasm_stock_id, $CVTERM['femal_type']['type_id'], $stock['maternal_parent']);
        }
      }

      //  check paternal parent exist in db
      if($stock['paternal_parent']){
        $match = array(
          'name' => $stock['paternal_parent'],
          'organism_id' => $organism
        );
        $results = chado_select_record('stock', ['stock_id'], $match);
        $paternal_parent_stock_id = $results[0]->stock_id;

        if (count($results) == 0){
          // relationship with paternal parent should not exist, check it
          $result_sr_not_exist = chado_select_record('stock_relationship', ['stock_relationship_id'], ['object_id'  => $germplasm_stock_id, 'type_id' => $CVTERM['paternal_parent']['type_id'] ]);
          if (count($result_sr_not_exist) != 0){
            $this->logMessage('WARNING: Paternal parent for !stock_relationship NOT exist in DB but parent relationship does.', ['!stock_relationship' => $stock['name']], TRIPAL_WARNING);
          }
        }
        elseif (count($results) == 1){
          //  load paternal parent to table stock_relationship
          $this->load_stock_relationship($stock['name'], $paternal_parent_stock_id, $germplasm_stock_id, $CVTERM['paternal_parent']['type_id']);
          //  load 'female parent' as a property to table:stockprop
          //$shit->load_stockprop($stock['name'], $germplasm_stock_id, $CVTERM['male_type']['type_id'], $stock['paternal_parent']);
        }
      }

    }//end of fget loop through uploaded file
    return [$file_path, $organism, $user_prefix];
 }// end of function loadGermplasm

/** check all cv required in this module
 *  after insertion/checking, update type_id(cvterm_id), cv_id, dbxref_id to array $CV
 *  @param $CV
 */
public function check_and_insert_cv($CV){
  foreach($CV as $key_hr_name =>$one_cv){
     $result_cv = chado_select_record('cv', ['cv_id'], ['name'=>$one_cv['name'] ]);
     if (count($result_cv) == 0) {
       $result_cv = chado_insert_cv($one_cv['name'], $one_cv['definition']);
       $CV[$key_hr_name]['cv_id'] = $result_cv['cv_id'];
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
     if (count($result_cvterm) == 0) {
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


  /** a function used when a parent exists
  * use this function to load a maternal/paternal parent's info to table:stock_relationship
  * 1. germplasm and parent should have checked exist in table:stock
  * 2. check if this relationship has been inserted in table:stock_relationship
  * 3. insert if not
  * 4. report error if more than one matches found
  *
  * @param  $stock_re_name
  * @param  $stock_re_subject_id
  * @param  $stock_re_object_id
  * @param  $stock_re_type_id
  */
   public function load_stock_relationship($stock_re_name, $stock_re_subject_id, $stock_re_object_id, $stock_re_type_id){

      $match = array(
       'object_id'  => $stock_re_object_id,
       'type_id' => $stock_re_type_id
     );
     $results_stock_rs_exist = chado_select_record('stock_relationship', ['stock_relationship_id', 'subject_id'], $match);

     $match = array(
       'subject_id' => $stock_re_subject_id,
       'object_id'  => $stock_re_object_id,
       'type_id' => $stock_re_type_id
     );
     $results_stock_rs = chado_select_record('stock_relationship', ['stock_relationship_id'], $match);

     if (count($results_stock_rs_exist) == 0){
       $values = array(
         'subject_id' => $stock_re_subject_id,
         'object_id' => $stock_re_object_id,
         'type_id' => $stock_re_type_id
       );
       chado_insert_record('stock_relationship', $values);
     }
     elseif(count($results_stock_rs_exist) > 1){
       $this->logMessage('WARNING: !stock_relationship, has multiple same kind relationships in table:stock_relationship.', ['!stock_relationship' => $stock_re_name], TRIPAL_WARNING);
     }
     else{
       if (count($results_stock_rs) == 0){
          $result = chado_select_record('stock', ['name'], ['stock_id'=> $results_stock_rs_exist[0]->subject_id]);
          $this->logMessage('WARNING: !stock_relationship has one relationship with !stock_parent but different from new parent in file.', ['!stock_relationship' => $stock_re_name, '!stock_parent' => $result[0]->name], TRIPAL_WARNING);
       }
     }
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
}// end of "class ExampleImporter extends TripalImporter"
