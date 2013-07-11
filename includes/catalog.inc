<?php
/**
 * @file
 * @copyright The Royal National Theatre
 * @author John-Paul Drawneek <jdrawneek@nationaltheatre.org.uk>
 */

/**
 * Import names form.
 *
 * @param array $node
 *   Structure of form
 * @param array $form_state
 *   Current structure and data of form
 *
 * @return array
 *   Updated form structure
 */
function calm_import_catalog($node, &$form_state) {
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
 * Import Catalog form validation.
 *
 * @param array $form
 *   Form structure
 * @param array $form_state
 *   Current structure and data of form
 */
function calm_import_catalog_validate($form, &$form_state) {
  _calm_import_form_validate($form, $form_state, 'Catalog');
}
/**
 * Process Import Catalog Form.
 *
 * @param array $form
 *   Form structure
 * @param array $form_state
 *   Current form data and structure
 */
function calm_import_catalog_submit($form, &$form_state) {
  _calm_import_form_submit(
    $form,
    $form_state,
    '_calm_import_save_cache_catalog',
    'Catalog'
  );
}
/**
 * Save Catalog to cache tables.
 *
 * Only saves items on the level of Item, as rest is structure for Calm.
 * @access private
 *
 * @param DOMElement $v
 *   Data to work on
 * @param int $counter
 *   Universal counter to track the number of items added to cache
 * @param int $updated
 *   Universal counter to track the number of items updated in cache
 */
function _calm_import_save_cache_catalog(DOMElement $v, &$counter, &$updated) {
  if (isset($v->getElementsByTagName('Level')->item(0)->nodeValue) &&
    strtolower($v->getElementsByTagName('Level')->item(0)->nodeValue) == 'item') {
    $result = db_merge('calm_cache_catalogue')
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
        'Type' => $v->getElementsByTagName('RecordType')->item(0)->nodeValue,
        'Series' => isset($v->getElementsByTagName('Series')->item(0)->nodeValue) ? $v->getElementsByTagName('Series')->item(0)->nodeValue : '',
        'RecordDump' => $v->C14N(FALSE, FALSE),
      ))
      ->execute();
    if ($result == MergeQuery::STATUS_INSERT) {
      $counter++;
    }
    elseif ($result == MergeQuery::STATUS_UPDATE) {
      $updated++;
    }
  }
}
