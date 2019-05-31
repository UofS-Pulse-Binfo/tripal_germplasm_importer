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
      <li>Institute Code: The code for the Institute that has bred the material.</li>
      <li>Institute Name: The name of the Institute that has bred the material.</li>
      <li>Pedigree: The cross name and optional selection history.</li>
      <li>Country of Origin Code: </li>
      <li>Synonyms: The synonyms of the accession.</li>
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
      '#description' => 'Please give a prefix for this germplasm accession. Unique names for germplasm contained in file will be this prefix followed by stock id. Unique prefix can help you track this germplasm in the future, and default prefix is GERM.<br><b>NOTE:</b> unique names for existed germplasm accession will be updated with new prefix too.',
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

    $this->loadGermplasmAccession($file_path, $organism_id, $user_prefix, $dbxref_id = NULL, $description = NULL, $is_obsolete = f);
  }

  public function loadGermplasmAccession($file_path, $organism, $user_prefix, $dbxref_id, $description, $is_obsolete){
    // print upload file name in Job LOGS
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
    )
    //read cvterm
    $CVTERM = array(
      'accession_number' => array(
        'name' => 'accession number',
        'cv_name' => 'multicrop passport ontology',
        'need_insert_by_function' => '0'
      ),
      'germplasm_genus' => array(
        'name' => 'genus',
        'cv_name' => 'multicrop passport ontology',
        'need_insert_by_function' => '0'
      ),
      'germplasm_species' => array(
        'name' => 'species',
        'cv_name' => 'multicrop passport ontology',
        'need_insert_by_function' => '0'
      ),
      'germplasm_name' => array(
        'name' => 'accession name',
        'cv_name' => 'multicrop passport ontology',
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
        'cv_name' => 'multicrop passport ontology',
        'need_insert_by_function' => '0'
      ),
      'institute_name' => array(
        'name' => 'Breeding institute name',
        'cv_name' => 'multicrop passport ontology',
        'need_insert_by_function' => '0'
      ),
      'pedigree' => array(
        'name' => 'pedigree information',
        'cv_name' => 'GCP germplasm ontology',
        'need_insert_by_function' => '0'
      ),
      'country_of_origin_code' => array(
        'name' => 'country of origin',
        'cv_name' => 'GCP germplasm ontology',
        'need_insert_by_function' => '0'
      ),
      'synonyms' => array(
        'name' => 'synonym',
        'cv_name' => 'GCP germplasm ontology',
        'need_insert_by_function' => '0'
      ),
      'default_display_name' => array(
        'name' => 'common crop name',
        'cv_name' => 'multicrop passport ontology',
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
        'cv_name' => 'multicrop passport ontology',
        'need_insert_by_function' => '0'
      ),
      'biological_status_of_accession_code' => array(
        'name' => 'biological status of accession',
        'cv_name' => 'GCP germplasm ontology',
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
        'cv_name' => 'multicrop passport ontology',
        'need_insert_by_function' => '0'
      ),
      'type_of_germplasm_storage_code' => array(
        'name' => 'type of germplasm storage',
        'cv_name' => 'GCP germplasm ontology',
        'need_insert_by_function' => '0'
      )
    );

    //database various so we need names for both cvterm and cv to determine specific cvterm id for a cvterm
    //?????? $CV = $this->check_and_insert_cv($CV);

    $CVTERM = $this->check_and_insert_cvterm($CVTERM, $CV);


    $Header_names = array('' => , );












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

  }
}
