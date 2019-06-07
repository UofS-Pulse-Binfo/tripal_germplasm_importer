<?php
namespace Tests;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;
use Faker\Factory;

class CrossImportingTest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
  use DBTransaction;

  /**
   * Basic test example.
   * Tests must begin with the word "test".
   * See https://phpunit.readthedocs.io/en/latest/ for more information.
   */
  /**
  * prepare all variables we need for test
  * using test file to insert some germplams corsses
  * use factory::create to generate organism
  */
  public function insert_test_file(){
    $faker = Factory::create();
    
    $file_path = DRUPAL_ROOT . '/' . drupal_get_path('module','tripal_germplasm_importer') . '/tests/test_files/unittest_sample_file.tsv';
    //alternative way: $arguments['file'][0]['file_path'] = __DIR__ . '/test_files/unittest_sample_file.tsv';
    
    $organism = factory('chado.organism')->create([
      'genus' => $faker->unique->word . uniqid(),
    ]);

    $organism_id = $organism->organism_id;

    $user_prefix = 'UnitTest';

    $importer = new \GermplasmCrossImporter();
    
    $result = $importer->loadGermplasmCross($file_path, $organism_id, $user_prefix, $dbxref_id = NULL, $description = NULL, $is_obsolete = f);
    
    return [
      'test_file_path' => trim($file_path, '"'),
      'organism_id' => $organism_id,
      'prefix' => $user_prefix,
    ];
  }

  /**
  * Test if insertion to table:stock is achieved
  * test if each germplasm is inserted with right uniquename, cvterm
  */
  public function testStockInsertion() {
    // load our module
    module_load_include('inc', 'tripal_germplasm_importer', 'includes/TripalImporter/GermplasmImporter');

    $insertion_result = $this -> insert_test_file();
    
    $this->assertTrue(true);
    
    // build an array for testing cvterms, key is the column number , value is the cvterm in chado:cvterm
    // key is the column number-1 to match array of explode line , value is the expected cvterm for each property
    $cvterm_term_check = array(
      '0' => 'crossingblock_year',
      '1' => 'crossingblock_season',
      '5' => 'additionalType',
      '6' => 'seed_type',
      '7' => 'cotyledon_colour',
      '8' => 'seed_coat_colour',
      '9' => 'comment',
    );

    // test for every germplasm line in test file
    $test_file = fopen(($insertion_result['test_file_path']), 'r');
    while ($line = fgets($test_file)) {
      
      $line = trim($line);
      
      if (preg_match("/Year/", $line)){continue;}

      $line_explode = explode("\t", $line);
      
      // test stock insertion in chado:stock
      $results = chado_select_record('stock', ['stock_id', 'uniquename', 'type_id'], ['name'=> $line_explode[2], 'organism_id'=> $insertion_result['organism_id'] ]);
      
      $this->assertEquals(1, count($results), "No or more than one $line_explode[2] in db chado:stock.");
      $germ_stock_id = $results[0]->stock_id;

      // test cvterm for germplams: F1
      $result = chado_select_record('cvterm', ['name'], ['cvterm_id'=>$results[0]->type_id]);
      $this->assertEquals('F1', $result[0]->name, "cvterm F1 does not match with cvterm in stock.");

      // test prefix of uniquename
      $this->assertEquals('0', strpos($results[0]->uniquename, $insertion_result['prefix']), "User input suffix is not updated in db.");
      
      // test properties need to insert to chado:stockprop (use $cvterm_term_check)
      foreach($cvterm_term_check as $key => $value){
	      $result = chado_select_record('stockprop', ['type_id'], ['stock_id'=>$germ_stock_id, 'value'=>$line_explode[$key]]);
	      $result = chado_select_record('cvterm', ['name'], ['cvterm_id'=>$result[0]->type_id]);
        $this->assertEquals($value, $result[0]->name, "cvterm $value does not match with cvterm in stockprop.");
      }

      // test relationship insertions in chado:stock_relationship
      // column 4, maternal parent
      $result = chado_select_record('stock', ['stock_id'], ['name'=> $line_explode[3], 'organism_id'=> $insertion_result['organism_id'] ]);
      if ($result){
	      $result = chado_select_record('stock_relationship', ['type_id'], ['subject_id'=>$result[0]->stock_id, 'object_id'=>$germ_stock_id]);
	      $result = chado_select_record('cvterm', ['name'], ['cvterm_id'=>$result[0]->type_id]);
        $this->assertEquals('is_maternal_parent_of', $result[0]->name, "cvterm is_maternal_parent_of does not match with cvterm in table:cvterm.");
      }

      // column 5, paternal parent
      $result = chado_select_record('stock', ['stock_id'], ['name'=> $line_explode[4], 'organism_id'=> $insertion_result['organism_id'] ]);
      if ($result){
	      $result = chado_select_record('stock_relationship', ['type_id'], ['subject_id'=>$result[0]->stock_id, 'object_id'=>$germ_stock_id]);
	      $result = chado_select_record('cvterm', ['name'], ['cvterm_id'=>$result[0]->type_id]);	
        $this->assertEquals('is_paternal_parent_of', $result[0]->name, "cvterm is_paternal_parent_of does not match with cvterm in table:cvterm.");
      }

    }
    fclose($test_file);
  }

}
