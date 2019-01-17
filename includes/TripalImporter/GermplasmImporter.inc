<?php

/**
 * @see TripalImporter
 */
 class GermplasmImporter extends TripalImporter {

  /**
   * The name of this loader.  This name will be presented to the site
   * user.
   */
  public static $name = 'Germplasm Importer';

  /**
   * The machine name for this loader. This name will be used to construct
   * the URL for the loader.
   */
  public static $machine_name = 'tripal_germplasm_importer';

  /**
   * A brief description for this loader.  This description will be
   * presented to the site user.
   */
  public static $description = 'import germplasm into Knowpulse';

  /**
   * An array containing the extensions of allowed file types.
   */
  public static $file_types = ['txt', 'tst', 'csv', 'tsv'];

  /**
   * Provides information to the user about the file upload.  Typically this
   * may include a description of the file types allowed.
   */
  public static $upload_description = 'Germplasm file should be a tab-delimited file containing multiple columns.';

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

    $form['organism_id'] = [
      '#type' => 'select',
      '#title' => t('Organism'),
      '#options' => $organisms,
      '#required' => TRUE,
      '#description' => t('Select the organism for germplasm file you would like to upload'),
    ];

    //Instructions
    $markup = '<h3> Welcome to Germplasm Importer</h3>
    <p> sth </p>
    <p> sth </p>
    <p> sth </p>';

    //Pick a CVterm

    $form['instruction'] = [
      '#markup' => $markup
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

    $this->loadGermplasm($analysis_id, $file_path, $organism_id, $dbxref_id = NULL, $description = NULL, $is_obsolete = f);
  }

  public function loadGermplasm($analysis_id, $file_path, $organism, $dbxref_id, $description, $is_obsolete){

    var_dump(["This is running!", $analysis_id, $file_path, $organism]);

    //???read cvterm
    $CVTERM = array(
      'cross_type' => array(
        'name' => 'F1',
        'is_obsolete' => 0,
        'is_relationshiptype' => 0,
        'definition' => 'The first generation of offspring from a parent cross.'
      ),
      'maternal_parent' => array(
        'name' => 'is_maternal_parent_of',
        'is_obsolete' => 0,
        'is_relationshiptype' => 0,
        'definition' => ''
      ),
      'paternal_parent' => array(
        'name' => 'is_paternal_parent_of',
        'is_obsolete' => 0,
        'is_relationshiptype' => 0,
        'definition' => ''
      ),
      'season' => array(
        'name' => 'crossingblock_season',
        'is_obsolete' => 0,
        'is_relationshiptype' => 0,
        'definition' => 'Indicates that the current stock was the progeny of a cross made in the season specified.'
      ),
      'year' => array(
        'name' => 'crossingblock_year',
        'is_obsolete' => 0,
        'is_relationshiptype' => 0,
        'definition' => 'Indicates that the current stock was the progeny of a cross made in the year specified.'
      ),
      'cotyledon_colour' => array(
        'name' => 'cotyledon_colour',
        'is_obsolete' => 0,
        'is_relationshiptype' => 0,
        'definition' => 'The colour of the cotyledon'
      ),
      'seed_type' => array(
        'name' => 'seed_type',
        'is_obsolete' => 0,
        'is_relationshiptype' => 0,
        'definition' => 'Characteristics or observable phenotypes of the seed.'
      ),
      'seed_coat_colour' => array(
        'name' => 'seed_coat_colour',
        'is_obsolete' => 0,
        'is_relationshiptype' => 0,
        'definition' => 'The colour of the seed coat'
      ),
      // double check with three blow when test on KP-dev
      'comment' => array(
        'name' => 'comment',
        'is_obsolete' => 0,
        'is_relationshiptype' => 0,
        'definition' => 'Comments, typically from users. '
      ),
      'femal_type' => array(
        'name' => 'femal_type',
        'is_obsolete' => 0,
        'is_relationshiptype' => 0,
        'definition' => ''
      ),
      'male_type' => array(
        'name' => 'male_type',
        'is_obsolete' => 0,
        'is_relationshiptype' => 0,
        'definition' => ''
      )

    );

    $CVTERM = $this->check_and_insert_cvterm($CVTERM);
    var_dump($CVTERM);
    //TO-DO: check table:cvterm and looking for femal_type / male_type
    //  no good match in KP chado.cvterm, also for seed_type
    //  temperately create there three terms

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

      //var_dump([$line_germplasm]);
      //From DOC (https://tripal.readthedocs.io/en/latest/dev_guide/custom_data_loader.html)
      // Calculate how many bytes we have read from the file and let the
      // importer know how many have been processed so it can provide a
      // progress indicator.
      $bytes_read += drupal_strlen($line);
      $this->setItemsHandled($bytes_read);

      $line_germplasm = trim($line_germplasm);
      $germ_col = explode(",", $line_germplasm);

      //header check, confirm column order
      //should be in order:
      //1.Year 2.Season 3.Cross No. 4.Maternal Parent 5.paternal Parent 6.Cross Type
      //7.Seed Type 8.Cotyledon Colour 9.Seed Coat
      if (preg_match('/^Year/', $line_germplasm) ){
        //check header specific names
	       var_dump(['Header line detected:']);
	       var_dump($germ_col);
        continue;
      }

      //For each germplasm lines
      //  since this organism is selected from drop-down box, so it exists in chado
      //  check maternal parent exist in db
      //  check paternal parent exist in db
      //chado api for check is chado_select_record($table, $columns, $values, $options = NULL)
      //chado api for insert is
      //save values into $array

      // skip comments
      if (preg_match('/^#/', $line_germplasm)) {
        continue;
      }

      // skip empty lines
      if (preg_match('/^\s*$/', $line_germplasm)) {
        continue;
      }

      $stock = array(
        'name' => $germ_col[2]
      );
      if($germ_col[0]){
        $stock['year'] = $germ_col[0];
      }
      if($germ_col[1]){
        $stock['season'] = $germ_col[1];
      }
      if($germ_col[3]){
        $stock['maternal_parent'] = $germ_col[3];
      }
      if($germ_col[4]){
        $stock['paternal_parent'] = $germ_col[4];
      }
      if($germ_col[5]){
        $stock['cross_type'] = $germ_col[5];
      }
      if($germ_col[6]){
        $stock['seed_type'] = $germ_col[6];
      }
      if($germ_col[7]){
        $stock['cotyledon_color'] = $germ_col[7];
      }
      if($germ_col[8]){
        $stock['seed_coat_color'] = $germ_col[8];
      }
      if($germ_col[9]){
        $stock['comment'] = $germ_col[9];
      }
      if($germ_col[10]){
        $stock['male_type'] = $germ_col[10];
      }
      if($germ_col[11]){
        $stock['female_type'] = $germ_col[11];
      }
      // TO-DO
      // male_type, female_type, commont
      var_dump($stock);
      //************************************************************************

      //FIRSRT: load to table Stock
      //check this germplasm name not exist in db
      // report error is yes
      $match = array(
        'name' => $stock['name'],
        'organism_id' => $organism
      );
      $results = chado_select_record('stock', ['stock_id','name'], $match);

      if (count($results) != 0){
        if (count($results) == 1){
          $this->logMessage('The stock, !stock, has one matched name in chado.stock.',['!stock' => $stock['name']], TRIPAL_WARNING);
          $germplasm_stock_id = $results[0]->stock_id;
        }
        else{
          $this->logMessage('ERROR: The stock, !stock, has more than one matched names in chado.stock.',['!stock' => $stock['name']], TRIPAL_WARNING);
        }
      }
      else{
        // then try to insert this germplasm into table:stock
        $this->logMessage('The stock, !stock, has no matched matched name in chado.stock.',['!stock' => $stock['name']], TRIPAL_WARNING);
        $value = array(
          'organism_id' => $organism,
          'name' => $stock['name'],
          'uniquename' => uniqid("Germplasm:"),
          'type_id' => [
            'name'=>'F1'
            ],
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
          print('ERROR:Import of germplasm '.$stock['name'].' FAILED!!!'."\n");
        }elseif (count($results) == 1){
          //  save stock_id for next step
          $germplasm_stock_id = $results[0]->stock_id;
          //  TO-DO: also save to log message
        }
      }


      //  load all extra info to table:stockprop
      //  use array $CVTERM and array $stock to match (names from file header) / (variable name) / (database cvterm)
      // loop though all keys in array $stock
      foreach($stock as $key=> $value){
        // check if this key exists in array $CVTERM
        if (($key == 'name') || ($key == 'maternal_parent') || ($key == 'paternal_parent')){
          continue;
        }

        if ($CVTERM[$key]){
          // for test, delete after
          echo $key . ' for ' . $CVTERM[$key]['name'] . "\n";
          // check if this cvterm exist in table:cvterm
          // we should have cheked this cvterm, double check just for sure
          $match = array(
            'name' => $CVTERM[$key]['name']
          );
          $results = chado_select_record('cvterm', ['cvterm_id'], $match);
          if ($results == 0){
            // for test, should be log message???
            $this->logMessage('ERROR: Cvterm , !cvterm, does not exist in databas table:cvterm.', ['!cvterm' => $CVTERM[$key]['name']], TRIPAL_WARNING);
          }
          else{
            //  use function to check/load this property to talbe:stockprop
            $this->load_stockprop($key, $germplasm_stock_id, $CVTERM[$key]['type_id'], $value);
          }
        }
        else{
          $this->logMessage('Column name , !header_name, does not exist in array CVTERM.', ['!header_name' => $key], TRIPAL_WARNING);
        }

      }

      //TO-DO: check if insertion is successful

      //  check maternal parent exist in db
      $match = array(
        'name' => $stock['maternal_parent'],
        'organism_id' => $organism
      );
      $results = chado_select_record('stock', ['stock_id'], $match);
      $maternal_parent_stock_id = $results[0]->stock_id;

      if (count($results) == 0){
        $this->logMessage('The stock, !stock, of maternal parent does not exist in the database',['!stock' => $stock['maternal_parent']], TRIPAL_WARNING);
        //continue;
      }
      elseif (count($results) == 1){
        // load maternal parent to table stock_relationship
        $this->load_stock_relationship($stock['maternal_parent'], $maternal_parent_stock_id, $germplasm_stock_id, $CVTERM['maternal_parent']['type_id']);

        //  also load 'femal type' to table:stockprop
        //  load_stockprop($stockprop_name, $stockprop_stock_id, $stockprop_type_id, $stockprop_value)
        if ($stock['female_type']){
          $this->load_stockprop($stock['maternal_parent'], $maternal_parent_stock_id, $CVTERM['seed_type']['type_id'], $stock['female_type']);
        }
      }

      //  check paternal parent exist in db
      $match = array(
        'name' => $stock['paternal_parent'],
        'organism_id' => $organism
      );
      $results = chado_select_record('stock', ['stock_id'], $match);
      $paternal_parent_stock_id = $results[0]->stock_id;

      if (count($results) == 0){
        $this->logMessage('The stock, !stock, of paternal parent does not exist in the database',['!stock' => $stock['paternal_parent']], TRIPAL_WARNING);
        //continue;
      }
      elseif (count($results) == 1){
        // load paternal parent to table stock_relationship
        $this->load_stock_relationship($stock['paternal_parent'], $paternal_parent_stock_id, $germplasm_stock_id, $CVTERM['paternal_parent']['type_id']);

        //  also load 'male type' to table:stockprop
        //  load_stockprop($stockprop_name, $stockprop_stock_id, $stockprop_type_id, $stockprop_value)
        if ($stock['male_type']){
          $this->load_stockprop($stock['paternal_parent'], $paternal_parent_stock_id, $CVTERM['seed_type']['type_id'], $stock['male_type']);
        }
      }

    }//end of fget loop through uploaded file
    return 1;
 }// end of function loadGermplasm


 /** check all cvterms required in this module
 * Three situatioins for each:
 *    1. only one match, use it, and save cvterm_id for further use as type_id
 *    2. no match, insert this term using chado api chado_insert_cvterm()
 *    3. more than one match ?????????????????????? TO-DO
 *      for now, check KP.chado.cvterm, find cv for this cvterm and make in unique
 *  after insertion/checking, update type_id(cvterm_id), cv_id, dbxref_id to array $CVTERM
 *  @param $CVTERM
 */
 public function check_and_insert_cvterm($CVTERM){
   //check each cvterm
   foreach($CVTERM as $key_hr_name =>$one_cvterm){
     $result_cvterm = chado_select_record('cvterm', ['cvterm_id','cv_id', 'dbxref_id'], ['name'=>$one_cvterm['name']]);

     if (count($result_cvterm) == 0) {
       $term = array(
         'name' => $one_cvterm['name'],
         'is_obsolete' => $one_cvterm['is_obsolete'],
         'is_relationshiptype' => $one_cvterm['is_relationshiptype'],
         'definition' => $one_cvterm['definition']
       );
       chado_insert_cvterm($term);
       $result_cvterm = chado_select_record('cvterm', ['cvterm_id','cv_id', 'dbxref_id'], ['name'=>$one_cvterm['name']]);
     }
     elseif(count($result_cvterm) == 1){
       $this->logMessage('The cvterm, !cvterm, has one match in chado.cvterm',['!cvterm' => $one_cvterm['name']], TRIPAL_WARNING);
     }
     else{
       $this->logMessage('WARNING: The cvterm, !cvterm, has more than one match in chado.cvterm',['!cvterm' => $one_cvterm['name']], TRIPAL_WARNING);
     }
     $CVTERM[$key_hr_name]['type_id'] = $result_cvterm[0]->cvterm_id;
     $CVTERM[$key_hr_name]['cv_id'] = $result_cvterm[0]->cv_id;
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
       'subject_id' => $stock_re_subject_id,
       'object_id'  => $stock_re_object_id,
       'type_id' => $stock_re_type_id
     );
     $results_stock_rs = chado_select_record('stock_relationship', 'stock_relationship_id', $match);
     if (count($results_stock_rs) == 0){
       // for test, delete after
       echo 'Talbe:stock_relationship insertion for ' . $stock_re_name . ':subject_id: ' . $stockprop_stock_id . ';object_id ' . $stockprop_type_id . ';type_id: ' . $stockprop_value . "\n";
       $values = array(
         'subject_id' => $stock_re_subject_id,
         'object_id' => $stock_re_object_id,
         'type_id' => $stock_re_type_id
       );
       chado_insert_record('stock_relationship', $values);
     }
     elseif(count($results_stock_rs) == 1){
       $this->logMessage('Stock relationship , !stock_relationship, exists in table:stock_relationship already.', ['!stock_relationship' => $stock_re_name], TRIPAL_WARNING);
     }
     else{
       // report for error, this property for this stock should only have one match
       $this->logMessage('ERROE: Stock relationship , !stock_relationship, has multiple matches in table:stock_relationship.', ['!stock_relationship' => $stock_re_name], TRIPAL_WARNING);
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
       // for test, delete after
       echo 'Talbe:stockprop insertion. stock_id: ' . $stockprop_stock_id . ';type_id: ' . $stockprop_type_id . ';value: ' . $stockprop_value . "\n";
       $values = array(
         'stock_id' => $stockprop_stock_id,
         'type_id' => $stockprop_type_id,
         'value' => $stockprop_value
       );
       $results = chado_insert_record('stockprop', $values);
     }
     elseif(count($result_stockprop) == 1){
       // for test, delete after
       $this->logMessage('Stock property, !stockprop, exists in table:stockprop already.', ['!stockprop' => $stockprop_name], TRIPAL_WARNING);
     }
     else{
       // report for error, this property for this stock should only have one match
       $this->logMessage('Stock property, !stockprop, has multiple matches in table:stockprop.', ['!stockprop' => $stockprop_name], TRIPAL_WARNING);
     }
   }
}// end of "class ExampleImporter extends TripalImporter"
