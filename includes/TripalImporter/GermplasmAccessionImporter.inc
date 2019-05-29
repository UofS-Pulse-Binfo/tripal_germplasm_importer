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
      <li>Year: the year the accession was made in.</li>
      <li>Season: the season the accession was made in (e.g. Spring, Fall, Winter, Summer).</li>
      <li>accession Number: a unique identifier for the accession (e.g. 1234S).</li>
      <li>Maternal Parent: the name of the maternal parent of this accession.</li>
      <li>Paternal Parent: the name of the paternal parent of this accession.</li>
      <li>accession Type: the type of accession (e.g. single, double, triple).</li>
      <li>Seed Type: either the market class or seed coat colour of the seed resulting from the accession.</li>
      <li>Cotyledon Colour: the cotyledon colour of the seed resulting from the accession.</li>
      <li>Comment: a free-text comment about the accession.</li>
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
  }
}
