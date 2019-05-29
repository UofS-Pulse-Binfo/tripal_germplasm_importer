<?php
/**
 * @file
 * This file contains the Loading Ontologies Tripal Job.
 * This file is copied from repository UofS-Pulse-Binfo/kp_entities:
 * (https://github.com/UofS-Pulse-Binfo/kp_entities/blob/master/includes/job.load_ontolgies.inc)
 */

/**
 * Load Ontologies
 */
function load_ontologies_job($job = NULL) {

  $ontology_path = drupal_get_path('module', 'kp_entities') . '/ontologies/';
  $ontologies = kp_entities_ontology_list();
  foreach ($ontologies as $abbrev => $details) {

    // If indicated then we want to load all terms for this ontology.
    if ($details['all terms']) {

      // The OBO loader is aided by a list of ontologies that can be loaded.
      // First we should make sure that all our ontologies are in that list :-).
      $obo_id = db_query('SELECT obo_id FROM {tripal_cv_obo} WHERE name=:name',
        array(':name' => $details['name']))->fetchField();
      if (!$obo_id) {
        $obo_id = db_insert('tripal_cv_obo')
          ->fields(array(
            'name' => $details['name'],
            'path' => $ontology_path . $details['file'],
          ))->execute();
      }

      // Check to see if the ontology has already been loaded :-).
      // Tripal names the cv's based on their namespace.
      // Thus we need to find the default namespace for each ontology.
      // Use grep to pull out the line with the default namespace.
      $command = 'grep "default-namespace" ' . $ontology_path . $details['file'];
      $line = shell_exec($command);

      // Now we extract the name from the line.
      preg_match('/default-namespace: (.*)$/', $line, $matches);
      $namespace = trim($matches[1]);

      // Now check to see if the controlled vocabulary is already loaded.
      $is_present = chado_query('SELECT true FROM {cv} WHERE name=:name',
        array(':name' => $namespace))->fetchField();

      // If not, submit the loading job.
      if (!$is_present) {
        tripal_submit_obo_job(array('obo_id' => $obo_id));
      }
    }
    // Otherwise we only want to insert a couple of terms.
    else {

      // First add the cv.
      tripal_insert_cv($details['name'], $details['definition']);

      // Now add each term.
      foreach ($details['terms'] as $term) {
        $term['cv_name'] = $details['name'];
        tripal_insert_cvterm($term);
      }
    }
  }
}
