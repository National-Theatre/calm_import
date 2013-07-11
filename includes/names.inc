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
function calm_import_names($node, &$form_state) {
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
 * Import names form validation.
 *
 * @param array $form
 *   Form structure
 * @param array $form_state
 *   Current structure and data of form
 */
function calm_import_names_validate($form, &$form_state) {
  _calm_import_form_validate($form, $form_state, 'Persons');
}
/**
 * Process Import Names Form.
 *
 * @param array $form
 *   Form structure
 * @param array $form_state
 *   Current form data and structure
 */
function calm_import_names_submit($form, &$form_state) {
  _calm_import_form_submit(
    $form,
    $form_state,
    '_calm_import_save_cache_names',
    'Names'
  );
}
/**
 * Save Names to cache tables.
 * @access private
 *
 * @param DOMNode $v
 *   Data to work on
 * @param int $counter
 *   Universal counter to track the number of items added to cache
 * @param int $updated
 *   Universal counter to track the number of items updated in cache
 */
function _calm_import_save_cache_names($v, &$counter, &$updated) {
  $extra_data = array();

  for ($index = 1; $index < 11; $index++) {
    $date = '';
    if (isset($v->getElementsByTagName('BioPublicationDate' . $index)->item(0)
                ->nodeValue)) {
      if (is_numeric($v->getElementsByTagName('BioPublicationDate' . $index)
                       ->item(0)->nodeValue)) {
        $date
          = $v->getElementsByTagName('BioPublicationDate' . $index)->item(0)
              ->nodeValue
          . '-01-01';
      }
      else {
        $date
          = $v->getElementsByTagName('BioPublicationDate' . $index)->item(0)
              ->nodeValue;
      }
    }
    $extra_data['BioPublication'][] = array(
      'link' => isset($v->getElementsByTagName('BioPublicationLink' . $index)->item(0)->nodeValue) ? $v->getElementsByTagName('BioPublicationLink' . $index)->item(0)->nodeValue : '',
      'text' => isset($v->getElementsByTagName('BioPublicationText' . $index)->item(0)->nodeValue) ? $v->getElementsByTagName('BioPublicationText' . $index)->item(0)->nodeValue : '',
      'publisher' => isset($v->getElementsByTagName('BioPublisher' . $index)->item(0)->nodeValue) ? $v->getElementsByTagName('BioPublisher' . $index)->item(0)->nodeValue : '',
      'date' => $date,
    );
  }
  for ($index = 1; $index < 4; $index++) {
    $extra_data['UnionBio'][] = array(
      'link' => isset($v->getElementsByTagName('UnionBioLink' . $index)->item(0)->nodeValue) ? $v->getElementsByTagName('UnionBioLink' . $index)->item(0)->nodeValue : '',
      'text' => isset($v->getElementsByTagName('UnionBioText' . $index)->item(0)->nodeValue) ? $v->getElementsByTagName('UnionBioText' . $index)->item(0)->nodeValue : '',
      'code' => isset($v->getElementsByTagName('UnionBioCode' . $index)->item(0)->nodeValue) ? $v->getElementsByTagName('UnionBioCode' . $index)->item(0)->nodeValue : '',
    );
  }
  if (isset($v->getElementsByTagName('AuthorityImage')->item(0)->nodeValue)) {
    $extra_data['AuthorityImage']
      = $v->getElementsByTagName('AuthorityImage')->item(0)->nodeValue;
  }
  else {
    $extra_data['AuthorityImage'] = '';
  }
  $result = db_merge('calm_cache_names')
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
            'NameCode' => $v->getElementsByTagName('Code')->item(0)->nodeValue,
            'PersonName' => $v->getElementsByTagName('PersonName')->item(0)->nodeValue,
            'Surname' => isset($v->getElementsByTagName('Surname')->item(0)->nodeValue) ? $v->getElementsByTagName('Surname')->item(0)->nodeValue : '',
            'Forenames' => isset($v->getElementsByTagName('Forenames')->item(0)->nodeValue) ? $v->getElementsByTagName('Forenames')->item(0)->nodeValue : '',
            'PreTitle' => isset($v->getElementsByTagName('PreTitle')->item(0)->nodeValue) ? $v->getElementsByTagName('PreTitle')->item(0)->nodeValue : '',
            'Title' => isset($v->getElementsByTagName('Title')->item(0)->nodeValue) ? $v->getElementsByTagName('Title')->item(0)->nodeValue : '',
            'Dates' => isset($v->getElementsByTagName('Dates')->item(0)->nodeValue) ? $v->getElementsByTagName('Dates')->item(0)->nodeValue : '',
            'Epithet' => isset($v->getElementsByTagName('Epithet')->item(0)->nodeValue) ? $v->getElementsByTagName('Epithet')->item(0)->nodeValue : '',
            'Gender' => isset($v->getElementsByTagName('Gender')->item(0)->nodeValue) ? $v->getElementsByTagName('Gender')->item(0)->nodeValue : '',
            'ParallelEntry' => $v->getElementsByTagName('ParallelEntry')->item(0)->nodeValue,
            'NonPreferredTerm' => $v->getElementsByTagName('NonPreferredTerm')->item(0)->nodeValue,
            'INFORMATIONAREA' => $v->getElementsByTagName('INFORMATIONAREA')->item(0)->nodeValue,
            'DatesAndPlaces' => $v->getElementsByTagName('DatesAndPlaces')->item(0)->nodeValue,
            'Address' => isset($v->getElementsByTagName('Address')->item(0)->nodeValue) ? $v->getElementsByTagName('Address')->item(0)->nodeValue : '',
            'Nationality' => isset($v->getElementsByTagName('Nationality')->item(0)->nodeValue) ? $v->getElementsByTagName('Nationality')->item(0)->nodeValue : '',
            'Activity' => $v->getElementsByTagName('Activity')->item(0)->nodeValue,
            'Relationships' => $v->getElementsByTagName('Relationships')->item(0)->nodeValue,
            'PublishedWorks' => isset($v->getElementsByTagName('PublishedWorks')->item(0)->nodeValue) ? $v->getElementsByTagName('PublishedWorks')->item(0)->nodeValue : '',
            'OtherInfo' => $v->getElementsByTagName('OtherInfo')->item(0)->nodeValue,
            'Source' => $v->getElementsByTagName('Source')->item(0)->nodeValue,
            'Conventions' => isset($v->getElementsByTagName('Conventions')->item(0)->nodeValue) ? $v->getElementsByTagName('Conventions')->item(0)->nodeValue : '',
            'PublicView' => isset($v->getElementsByTagName('PublicView')->item(0)->nodeValue) ? $v->getElementsByTagName('PublicView')->item(0)->nodeValue : '',
            'nSet' => isset($v->getElementsByTagName('Set')->item(0)->nodeValue) ? $v->getElementsByTagName('Set')->item(0)->nodeValue : '',
            'ExtraData' => serialize($extra_data),
          ))
          ->execute();
  if ($result == MergeQuery::STATUS_INSERT) {
    $counter++;
  }
  elseif ($result == MergeQuery::STATUS_UPDATE) {
    $updated++;
  }
}
