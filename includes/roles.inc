<?php
/**
 * @file
 * @copyright The Royal National Theatre
 * @author John-Paul Drawneek <jdrawneek@nationaltheatre.org.uk>
 */

/**
 * Role import form.
 *
 * @param array $node
 *   Form structure
 * @param array $form_state
 *   Current form data
 *
 * @return array
 *   Updated form structure
 */
function calm_import_roles($node, &$form_state) {
  $item = array();

  $item['xml'] = array(
    '#type' => 'file',
    '#title' => 'Xml File',
  );

  $item['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
  );

  return $item;
}
/**
 * Validate Import Roles Form.
 *
 * @param array $form
 *   Form structure
 * @param array $form_state
 *   Current form data and structure
 */
function calm_import_roles_validate($form, &$form_state) {
  $validators = array('file_validate_extensions' => array('xml'));

  if (defined('TESTING')) {
    print "testing mode\n\n";
    $file = file_save_upload_hack('xml', $validators, "temporary://");
  }
  else {
    $file = file_save_upload('xml', $validators, "temporary://");
  }
  if ($file) {
    $prev = libxml_use_internal_errors(TRUE);
    $result = simplexml_load_file(drupal_realpath($file->uri));
    if ($result === FALSE) {
      watchdog('CALM Import', '<pre>' . print_r(libxml_get_errors(), TRUE) . '</pre>');
      form_set_error('xml', 'File uploaded does not contain valid xml.');
      file_delete($file);
    }
    else {
      $form_state['uploaded_file'] = $file;
    }
    libxml_use_internal_errors($prev);
  }
  else {
    form_set_error('xml', 'Not an xml file uploaded.');
  }
}
/**
 * Process Import Role Form.
 *
 * @param array $form
 *   Form structure
 * @param array $form_state
 *   Current form data and structure
 */
function calm_import_roles_submit($form, &$form_state) {
  $lock = variable_get('calm_import_perf_lock', FALSE);
  if ($lock) {
    drupal_set_message('Already uploading a file', 'warning');
  }
  else {
    variable_set('calm_import_perf_lock', TRUE);
    $file = $form_state['uploaded_file'];
    if (!file_exists(drupal_realpath('public://calmXml/'))) {
      drupal_mkdir('public://calmXml/');
    }
    $move_result = file_move($file, 'public://calmXml/' . $file->filename);
    $counter = 0;
    $updated = 0;
    if ($move_result === FALSE) {
      drupal_set_message('Failed to move xml uploaded to working directory', 'error');
      print print_r($_SESSION['messages'], TRUE) . "\n\n";
    }
    else {
      set_time_limit(0);
      $prev = libxml_use_internal_errors(TRUE);
      $dom = new DOMDocument();
      $dom->load(drupal_realpath($move_result->uri));
      $list = $dom->getElementsByTagName('DScribeRecord');
      foreach ($list as $i => $v) {
        _calm_import_save_cache_roles($v, $counter, $updated);
      }
      $errors = libxml_get_errors();
      if (!empty($errors)) {
        watchdog('CALM Import', '<pre>' . print_r($errors, TRUE) . '</pre>');
      }
      libxml_use_internal_errors($prev);
    }
    if ($counter > 0) {
      drupal_set_message('Added: ' . $counter . ' Roles');
    }
    if ($updated > 0) {
      drupal_set_message('Updated: ' . $updated . ' Roles');
    }
    variable_set('calm_import_perf_lock', FALSE);
  }
}
/**
 * Save roles to cache table.
 * @access private
 *
 * @param DOMNode $v
 *   Data to work on
 * @param int $counter
 *   Universal counter to track the number of items added to cache
 * @param int $updated
 *   Universal counter to track the number of items updated in cache
 */
function _calm_import_save_cache_roles($v, &$counter, &$updated) {
  $result = db_merge('calm_cache_roles')
          ->key(array(
            'RecordID' => $v->getElementsByTagName('RecordID')->item(0)->nodeValue,
          ))
          ->insertFields(array(
            'RecordID' => $v->getElementsByTagName('RecordID')->item(0)->nodeValue,
            'created' => time(),
            'changed' => time(),
          ))
          ->updateFields(array(
            'changed' => time(),
          ))
          ->fields(array(
            'RoleCode' => isset($v->getElementsByTagName('RoleCode')->item(0)->nodeValue) ? trim($v->getElementsByTagName('RoleCode')->item(0)->nodeValue) : '',
            'NameCode' => isset($v->getElementsByTagName('NameCode')->item(0)->nodeValue) ? trim($v->getElementsByTagName('NameCode')->item(0)->nodeValue) : '',
            'Role' => isset($v->getElementsByTagName('Role')->item(0)->nodeValue) ? trim($v->getElementsByTagName('Role')->item(0)->nodeValue) : '',
            'Name' => isset($v->getElementsByTagName('Name')->item(0)->nodeValue) ? trim($v->getElementsByTagName('Name')->item(0)->nodeValue) : '',
            'Note' => isset($v->getElementsByTagName('Note')->item(0)->nodeValue) ? trim($v->getElementsByTagName('Note')->item(0)->nodeValue) : '',
            'Dates' => isset($v->getElementsByTagName('Dates')->item(0)->nodeValue) ? trim($v->getElementsByTagName('Dates')->item(0)->nodeValue) : '',
          ))
          ->execute();
  if ($result == MergeQuery::STATUS_INSERT) {
    $counter++;
  }
  elseif ($result == MergeQuery::STATUS_UPDATE) {
    $updated++;
  }
}
