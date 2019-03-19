<?php
namespace Tests;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;
use Faker\Factory;

class ImportingTest extends TripalTestCase {
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
  * insert to a random germplasm??? in db  or use factory???
  */
  public function insert_test_file(){
    $faker = Factory::create();
    //loadGermplasm($file_path, $organism_id, $user_prefix, $dbxref_id = NULL, $description = NULL, $is_obsolete = f);
    $file_path = DRUPAL_ROOT . '/' . drupal_get_path('module','tripal_germplasm_importer') . '/tests/test_files/unittest_sample_file.tsv';

    $organism = factory('chado.organism')->create([
      'genus' => $faker->unique->word . uniqid(),
    ]);

    $organism_id = $organism->organism_id;

    $user_prefix = 'UnitTest_';

    $insertion_result = loadGermplasm($file_path, $organism_id, $user_prefix, $dbxref_id = NULL, $description = NULL, $is_obsolete = f);

    return [
      'final' => $insertion_results,
      'test_file_path' => $file_path,
      'organism_id' => $organism_id,
      'prefix' => $user_prefix,
    ];
  }

  /**
  * Test if insertion to table:stock is achieved
  * test if each germplasm is inserted with right uniquename, cvterm
  */
  public function testStockInsertion() {

    $insertion_results = $this -> insert_test_file();
    $this->assertTrue($insertion_result['final']);

    // build an array for testing cvterms, key is the column number , value is the cvterm in chado:cvterm
    $cvterm_term_check = array(
      '0' => 'crossingblock_year',
      '1' => 'crossingblock_season',
      '5' => 'additionalType',
      '6' => 'seed_type',
      '7' => 'cotyledon_colour',
      '8' => 'seed_coat_colour',
      '9' => 'comment',
    );

    //  test for every germplasm line in test file
    foreach (new SplFileObject($file_path) as $line) {
      //  test stock insertion in chado:stock
      trim($line);
      if (preg_match('Year'), $line){continue;}
      $line_explode = explode("\t", $line);

      $results = chado_select_record('stock', ['stock_id', 'uniquename', 'type_id'], ['name'=> $line_explode[2], 'organism_id'=> $insertion_result['organism_id'] ]);
      $this->assertEquals(1, count($results), "No or more than one $line_explode[2] in db chado:stock.");
      $germ_stock_id = $results[0]->stock_id;

      //  test cvterm for germplams: F1
      $result = chado_select_record('cvterm', ['name'], ['cvterm_id'=>$results[0]->type_id]);
      //$this->assertEquals(1, count($results), "No or more than one cvterm with cvterm_id $results[0]->type_id]");
      $this->assertEquals('F1', $result[0]->name, "cvterm F1 does not match with cvterm in stock.");

      //  test prefix of uniquename
      $this->assertArrayHasKey(preg_match($insertion_results['prefix'], $results[0]->uniquename), "User input suffix is not updated in db.")
      //  test property insertions in chado:stockprop, everyone?

      foreach($cvterm_term_check as $key => $value){
        $result = chado_select_record('stock_prop', ['type_id'], ['stock_id'=>$germ_stock_id, 'value'=>$line_explode[$key]]);
        $result = chado_select_record('cvterm', ['name'], ['cvterm_id'=>$result[0]->type_id]);
        $this->assertEquals($vaule, $result[0]->name, "cvterm $value does not match with cvterm in stockprop.");
      }

      //  test relationship insertions in chado:stock_relationship
      // column 4, maternal parent
      $result = chado_select_record('stock', ['stock_id'], ['name'=> $line_explode[3], 'organism_id'=> $insertion_result['organism_id'] ]);
      if ($result){
        $result = chado_select_record('stock_relationship', ['type_id'], ['subject_id'=>$result[0]->type_id, 'object_id'=>$germplasm_stock_id]);
        $this->assertEquals('is_maternal_parent_of', $result[0]->name, "cvterm is_maternal_parent_of does not match with cvterm in stock_relationship.");
      }

      // column 5, maternal parent
      $result = chado_select_record('stock', ['stock_id'], ['name'=> $line_explode[4], 'organism_id'=> $insertion_result['organism_id'] ]);
      if ($result){
        $result = chado_select_record('stock_relationship', ['type_id'], ['subject_id'=>$result[0]->type_id, 'object_id'=>$germplasm_stock_id]);
        $this->assertEquals('is_paternal_parent_of', $result[0]->name, "cvterm is_paternal_parent_of does not match with cvterm in stock_relationship.");
      }

    }
  }

}
