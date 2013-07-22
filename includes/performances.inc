<?php
/**
 * @file
 * @copyright The Royal National Theatre
 * @author John-Paul Drawneek <jdrawneek@nationaltheatre.org.uk>
 */

/**
 * Form to upload Performance XML data.
 *
 * @param array $form
 *   Form structure
 * @param array $form_state
 *   Current form data
 *
 * @return array
 *   Form to render
 */
function calm_import_performance($form, &$form_state) {
  $form = array();

  $form['xml'] = array(
    '#type' => 'file',
    '#title' => 'Xml File',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
  );

  return $form;
}
/**
 * Import form Validation.
 *
 * @param array $form
 *   Form structure
 * @param array $form_state
 *   Current form data
 */
function calm_import_performance_validate($form, &$form_state) {

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
 * Import Form Submit.
 *
 * @param array $form
 *   Form structure
 * @param array $form_state
 *   Current form data
 */
function calm_import_performance_submit($form, &$form_state) {
  $file = $form_state['uploaded_file'];
  if (!file_exists(drupal_realpath('public://calmXml/'))) {
    drupal_mkdir('public://calmXml/');
  }
  $move_result = file_move($file, 'public://calmXml/' . $file->filename);
  $counter = 0;
  $updated = 0;
  if ($move_result === FALSE) {
    drupal_set_message('Failed to move xml uploaded to working directory', 'error');
  }
  else {
    set_time_limit(0);
    $prev = libxml_use_internal_errors(TRUE);
    $dom = new DOMDocument();
    $dom->load(drupal_realpath($move_result->uri));
    $list = $dom->getElementsByTagName('DScribeRecord');
    foreach ($list as $i => $v) {
      _calm_import_save_cache_performance($v, $counter, $updated);
    }
    $errors = libxml_get_errors();
    if (!empty($errors)) {
      watchdog('CALM Import', '<pre>' . print_r($errors, TRUE) . '</pre>');
    }
    libxml_use_internal_errors($prev);
  }
  if ($counter > 0) {
    drupal_set_message('Added: ' . $counter . ' Productions');
  }
  if ($updated > 0) {
    drupal_set_message('Updated: ' . $updated . ' Productions');
  }
}

/**
 * Save performance to local cache.
 * @access private
 *
 * @param DOMElement $v
 *   Imported xml dom structure
 * @param int $counter
 *   holds the number of items inserted
 * @param int $updated
 *   holds the number of items updated
 */
function _calm_import_save_cache_performance(DOMElement $v, &$counter, &$updated) {
  $extra_data = array();

  for ($index = 1; $index < 31; $index++) {
    if (!isset($v->getElementsByTagName('UnionText' . $index)->item(0)->nodeValue)) {
      break;
    }
    $extra_data['Union'][] = array(
      'link' => isset($v->getElementsByTagName('UnionLink' . $index)->item(0)->nodeValue) ? $v->getElementsByTagName('UnionLink' . $index)->item(0)->nodeValue : '',
      'text' => isset($v->getElementsByTagName('UnionText' . $index)->item(0)->nodeValue) ? $v->getElementsByTagName('UnionText' . $index)->item(0)->nodeValue : '',
      'code' => isset($v->getElementsByTagName('UnionCode' . $index)->item(0)->nodeValue) ? $v->getElementsByTagName('UnionCode' . $index)->item(0)->nodeValue : '',
      'archive' => isset($v->getElementsByTagName('UnionArchive' . $index)->item(0)->nodeValue) ? $v->getElementsByTagName('UnionArchive' . $index)->item(0)->nodeValue : '',
    );
  }

  if (isset($v->getElementsByTagName('Synopsis')->item(0)->nodeValue)) {
    $synopsis = $v->getElementsByTagName('Synopsis')->item(0)->nodeValue;
  }
  else {
    $synopsis = '';
  }
  if (isset($v->getElementsByTagName('RightsInfo')->item(0)->nodeValue)) {
    $rights_info = $v->getElementsByTagName('RightsInfo')->item(0)->nodeValue;
  }
  else {
    $rights_info = '';
  }
  // @todo refactor this as below has got a bit messy.
  $result = db_merge('calm_cache_performance')
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
            'PerfCode' => isset($v->getElementsByTagName('PerfCode')->item(0)->nodeValue) ? $v->getElementsByTagName('PerfCode')->item(0)->nodeValue : '',
            'Title' => $v->getElementsByTagName('Title')->item(0)->nodeValue,
            'Title2' => isset($v->getElementsByTagName('UserText07')->item(0)->nodeValue) ? $v->getElementsByTagName('UserText07')->item(0)->nodeValue : '',
            'AltTitle' => isset($v->getElementsByTagName('AltTitle')->item(0)->nodeValue) ? $v->getElementsByTagName('AltTitle')->item(0)->nodeValue : '',
            'OriginalDate' => $v->getElementsByTagName('OriginalDate')->item(0)->nodeValue,
            'Venue' => $v->getElementsByTagName('Venue')->item(0)->nodeValue,
            'Venue2' => isset($v->getElementsByTagName('Venue2')->item(0)->nodeValue) ? $v->getElementsByTagName('Venue2')->item(0)->nodeValue : '',
            'UserText10' => $v->getElementsByTagName('UserText10')->item(0)->nodeValue,
            'Company' => $v->getElementsByTagName('Company')->item(0)->nodeValue,
            'Company2' => isset($v->getElementsByTagName('Company2')->item(0)->nodeValue) ? $v->getElementsByTagName('Company2')->item(0)->nodeValue : '',
            'Author' => isset($v->getElementsByTagName('Author')->item(0)->nodeValue) ? $v->getElementsByTagName('Author')->item(0)->nodeValue : '',
            'AuthorTwo' => isset($v->getElementsByTagName('AuthorTwo')->item(0)->nodeValue) ? $v->getElementsByTagName('AuthorTwo')->item(0)->nodeValue : '',
            'GenNotes' => isset($v->getElementsByTagName('GenNotes')->item(0)->nodeValue) ? $v->getElementsByTagName('GenNotes')->item(0)->nodeValue : '',
            'Synopsis' => $synopsis,
            'Setting' => isset($v->getElementsByTagName('Setting')->item(0)->nodeValue) ? $v->getElementsByTagName('Setting')->item(0)->nodeValue : '',
            'PerformanceType' => $v->getElementsByTagName('PerformanceType')->item(0)->nodeValue,
            'Town' => isset($v->getElementsByTagName('Town')->item(0)->nodeValue) ? $v->getElementsByTagName('Town')->item(0)->nodeValue : '',
            'NoActs' => isset($v->getElementsByTagName('NoActs')->item(0)->nodeValue) ? $v->getElementsByTagName('NoActs')->item(0)->nodeValue : 0,
            'OpeningNight' => $v->getElementsByTagName('OpeningNight')->item(0)->nodeValue,
            'PressNight' => $v->getElementsByTagName('PressNight')->item(0)->nodeValue,
            'LastPerformed' => $v->getElementsByTagName('LastPerformed')->item(0)->nodeValue,
            'NoPerformances' => $v->getElementsByTagName('NoPerformances')->item(0)->nodeValue,
            'MaleCast' => isset($v->getElementsByTagName('MaleCast')->item(0)->nodeValue) && is_numeric((int) $v->getElementsByTagName('MaleCast')->item(0)->nodeValue) ? (int) $v->getElementsByTagName('MaleCast')->item(0)->nodeValue : 0,
            'FemaleCast' => isset($v->getElementsByTagName('FemaleCast')->item(0)->nodeValue) && is_numeric((int) $v->getElementsByTagName('FemaleCast')->item(0)->nodeValue) ? (int) $v->getElementsByTagName('FemaleCast')->item(0)->nodeValue : 0,
            'CastSum' => isset($v->getElementsByTagName('CastSum')->item(0)->nodeValue) ? (is_numeric($v->getElementsByTagName('CastSum')->item(0)->nodeValue) ? $v->getElementsByTagName('CastSum')->item(0)->nodeValue : 0) : 0,
            'Awards' => $v->getElementsByTagName('Awards')->item(0)->nodeValue,
            'RightsInfo' => $rights_info,
            'IfFirstNonBrit' => isset($v->getElementsByTagName('IfFirstNonBrit')->item(0)->nodeValue) ? $v->getElementsByTagName('IfFirstNonBrit')->item(0)->nodeValue : 0,
            'DataSource' => isset($v->getElementsByTagName('DataSource')->item(0)->nodeValue) ? $v->getElementsByTagName('DataSource')->item(0)->nodeValue : '',
            'Keywords' => isset($v->getElementsByTagName('Keywords')->item(0)->nodeValue) ? $v->getElementsByTagName('Keywords')->item(0)->nodeValue : '',
            'Website' => isset($v->getElementsByTagName('Website')->item(0)->nodeValue) ? $v->getElementsByTagName('Website')->item(0)->nodeValue : '',
            'PlayTexts' => isset($v->getElementsByTagName('PlayTexts')->item(0)->nodeValue) ? $v->getElementsByTagName('PlayTexts')->item(0)->nodeValue : '',
            'PubLink1' => isset($v->getElementsByTagName('PubLink1')->item(0)->nodeValue) ? $v->getElementsByTagName('PubLink1')->item(0)->nodeValue : '',
            'PubText1' => isset($v->getElementsByTagName('PubText1')->item(0)->nodeValue) ? $v->getElementsByTagName('PubText1')->item(0)->nodeValue : '',
            'Pub1' => isset($v->getElementsByTagName('Pub1')->item(0)->nodeValue) ? $v->getElementsByTagName('Pub1')->item(0)->nodeValue : '',
            'PubDate1' => isset($v->getElementsByTagName('PubDate1')->item(0)->nodeValue) ? $v->getElementsByTagName('PubDate1')->item(0)->nodeValue : '',
            'PubLink2' => isset($v->getElementsByTagName('PubLink2')->item(0)->nodeValue) ? $v->getElementsByTagName('PubLink2')->item(0)->nodeValue : '',
            'PubText2' => isset($v->getElementsByTagName('PubText2')->item(0)->nodeValue) ? $v->getElementsByTagName('PubText2')->item(0)->nodeValue : '',
            'Pub2' => isset($v->getElementsByTagName('Pub2')->item(0)->nodeValue) ? $v->getElementsByTagName('Pub2')->item(0)->nodeValue : '',
            'PubDate2' => isset($v->getElementsByTagName('PubDate2')->item(0)->nodeValue) ? $v->getElementsByTagName('PubDate2')->item(0)->nodeValue : '',
            'PubLink3' => isset($v->getElementsByTagName('PubLink3')->item(0)->nodeValue) ? $v->getElementsByTagName('PubLink3')->item(0)->nodeValue : '',
            'PubText3' => isset($v->getElementsByTagName('PubText3')->item(0)->nodeValue) ? $v->getElementsByTagName('PubText3')->item(0)->nodeValue : '',
            'Pub3' => isset($v->getElementsByTagName('Pub3')->item(0)->nodeValue) ? $v->getElementsByTagName('Pub3')->item(0)->nodeValue : '',
            'PubDate3' => isset($v->getElementsByTagName('PubDate3')->item(0)->nodeValue) ? $v->getElementsByTagName('PubDate3')->item(0)->nodeValue : '',
            'PubLink4' => isset($v->getElementsByTagName('PubLink4')->item(0)->nodeValue) ? $v->getElementsByTagName('PubLink4')->item(0)->nodeValue : '',
            'PubText4' => isset($v->getElementsByTagName('PubText4')->item(0)->nodeValue) ? $v->getElementsByTagName('PubText4')->item(0)->nodeValue : '',
            'Pub4' => isset($v->getElementsByTagName('Pub4')->item(0)->nodeValue) ? $v->getElementsByTagName('Pub4')->item(0)->nodeValue : '',
            'PubDate4' => isset($v->getElementsByTagName('PubDate4')->item(0)->nodeValue) ? $v->getElementsByTagName('PubDate4')->item(0)->nodeValue : '',
            'PubLink5' => isset($v->getElementsByTagName('PubLink5')->item(0)->nodeValue) ? $v->getElementsByTagName('PubLink5')->item(0)->nodeValue : '',
            'PubText5' => isset($v->getElementsByTagName('PubText5')->item(0)->nodeValue) ? $v->getElementsByTagName('PubText5')->item(0)->nodeValue : '',
            'Pub5' => isset($v->getElementsByTagName('Pub5')->item(0)->nodeValue) ? $v->getElementsByTagName('Pub5')->item(0)->nodeValue : '',
            'PubDate5' => isset($v->getElementsByTagName('PubDate5')->item(0)->nodeValue) ? $v->getElementsByTagName('PubDate5')->item(0)->nodeValue : '',
            'AssetLink1' => isset($v->getElementsByTagName('AssetLink1')->item(0)->nodeValue) ? $v->getElementsByTagName('AssetLink1')->item(0)->nodeValue : '',
            'AssetText1' => isset($v->getElementsByTagName('AssetText1')->item(0)->nodeValue) ? $v->getElementsByTagName('AssetText1')->item(0)->nodeValue : '',
            'AssetCode1' => isset($v->getElementsByTagName('AssetCode1')->item(0)->nodeValue) ? $v->getElementsByTagName('AssetCode1')->item(0)->nodeValue : '',
            'AssetLink2' => isset($v->getElementsByTagName('AssetLink2')->item(0)->nodeValue) ? $v->getElementsByTagName('AssetLink2')->item(0)->nodeValue : '',
            'AssetText2' => isset($v->getElementsByTagName('AssetText2')->item(0)->nodeValue) ? $v->getElementsByTagName('AssetText2')->item(0)->nodeValue : '',
            'AssetCode2' => isset($v->getElementsByTagName('AssetCode2')->item(0)->nodeValue) ? $v->getElementsByTagName('AssetCode2')->item(0)->nodeValue : '',
            'AssetLink3' => isset($v->getElementsByTagName('AssetLink3')->item(0)->nodeValue) ? $v->getElementsByTagName('AssetLink3')->item(0)->nodeValue : '',
            'AssetText3' => isset($v->getElementsByTagName('AssetText3')->item(0)->nodeValue) ? $v->getElementsByTagName('AssetText3')->item(0)->nodeValue : '',
            'AssetCode3' => isset($v->getElementsByTagName('AssetCode3')->item(0)->nodeValue) ? $v->getElementsByTagName('AssetCode3')->item(0)->nodeValue : '',
            'AssetLink4' => isset($v->getElementsByTagName('AssetLink4')->item(0)->nodeValue) ? $v->getElementsByTagName('AssetLink4')->item(0)->nodeValue : '',
            'AssetText4' => isset($v->getElementsByTagName('AssetText4')->item(0)->nodeValue) ? $v->getElementsByTagName('AssetText4')->item(0)->nodeValue : '',
            'AssetCode4' => isset($v->getElementsByTagName('AssetCode4')->item(0)->nodeValue) ? $v->getElementsByTagName('AssetCode4')->item(0)->nodeValue : '',
            'AssetLink5' => isset($v->getElementsByTagName('AssetLink5')->item(0)->nodeValue) ? $v->getElementsByTagName('AssetLink5')->item(0)->nodeValue : '',
            'AssetText5' => isset($v->getElementsByTagName('AssetText5')->item(0)->nodeValue) ? $v->getElementsByTagName('AssetText5')->item(0)->nodeValue : '',
            'AssetCode5' => isset($v->getElementsByTagName('AssetCode5')->item(0)->nodeValue) ? $v->getElementsByTagName('AssetCode5')->item(0)->nodeValue : '',
            'UnionLink1' => isset($v->getElementsByTagName('UnionLink1')->item(0)->nodeValue) ? $v->getElementsByTagName('UnionLink1')->item(0)->nodeValue : '',
            'UnionText1' => isset($v->getElementsByTagName('UnionText1')->item(0)->nodeValue) ? $v->getElementsByTagName('UnionText1')->item(0)->nodeValue : '',
            'UnionCode1' => isset($v->getElementsByTagName('UnionCode1')->item(0)->nodeValue) ? $v->getElementsByTagName('UnionCode1')->item(0)->nodeValue : '',
            'UnionArchive1' => isset($v->getElementsByTagName('UnionArchive1')->item(0)->nodeValue) ? $v->getElementsByTagName('UnionArchive1')->item(0)->nodeValue : '',
            'UnionContact1' => isset($v->getElementsByTagName('UnionContact1')->item(0)->nodeValue) ? $v->getElementsByTagName('UnionContact1')->item(0)->nodeValue : '',
            'UnionLink2' => isset($v->getElementsByTagName('UnionLink2')->item(0)->nodeValue) ? $v->getElementsByTagName('UnionLink2')->item(0)->nodeValue : '',
            'UnionText2' => isset($v->getElementsByTagName('UnionText2')->item(0)->nodeValue) ? $v->getElementsByTagName('UnionText2')->item(0)->nodeValue : '',
            'UnionCode2' => isset($v->getElementsByTagName('UnionCode2')->item(0)->nodeValue) ? $v->getElementsByTagName('UnionCode2')->item(0)->nodeValue : '',
            'UnionArchive2' => isset($v->getElementsByTagName('UnionArchive2')->item(0)->nodeValue) ? $v->getElementsByTagName('UnionArchive2')->item(0)->nodeValue : '',
            'UnionContact2' => isset($v->getElementsByTagName('UnionContact2')->item(0)->nodeValue) ? $v->getElementsByTagName('UnionContact2')->item(0)->nodeValue : '',
            'UnionLink3' => isset($v->getElementsByTagName('UnionLink3')->item(0)->nodeValue) ? $v->getElementsByTagName('UnionLink3')->item(0)->nodeValue : '',
            'UnionText3' => isset($v->getElementsByTagName('UnionText3')->item(0)->nodeValue) ? $v->getElementsByTagName('UnionText3')->item(0)->nodeValue : '',
            'UnionCode3' => isset($v->getElementsByTagName('UnionCode3')->item(0)->nodeValue) ? $v->getElementsByTagName('UnionCode3')->item(0)->nodeValue : '',
            'UnionArchive3' => isset($v->getElementsByTagName('UnionArchive3')->item(0)->nodeValue) ? $v->getElementsByTagName('UnionArchive3')->item(0)->nodeValue : '',
            'UnionContact3' => isset($v->getElementsByTagName('UnionContact3')->item(0)->nodeValue) ? $v->getElementsByTagName('UnionContact3')->item(0)->nodeValue : '',
            'UnionLink4' => isset($v->getElementsByTagName('UnionLink4')->item(0)->nodeValue) ? $v->getElementsByTagName('UnionLink4')->item(0)->nodeValue : '',
            'UnionText4' => isset($v->getElementsByTagName('UnionText4')->item(0)->nodeValue) ? $v->getElementsByTagName('UnionText4')->item(0)->nodeValue : '',
            'UnionCode4' => isset($v->getElementsByTagName('UnionCode4')->item(0)->nodeValue) ? $v->getElementsByTagName('UnionCode4')->item(0)->nodeValue : '',
            'UnionArchive4' => isset($v->getElementsByTagName('UnionArchive4')->item(0)->nodeValue) ? $v->getElementsByTagName('UnionArchive4')->item(0)->nodeValue : '',
            'UnionContact4' => isset($v->getElementsByTagName('UnionContact4')->item(0)->nodeValue) ? $v->getElementsByTagName('UnionContact4')->item(0)->nodeValue : '',
            'UnionLink5' => isset($v->getElementsByTagName('UnionLink5')->item(0)->nodeValue) ? $v->getElementsByTagName('UnionLink5')->item(0)->nodeValue : '',
            'UnionText5' => isset($v->getElementsByTagName('UnionText5')->item(0)->nodeValue) ? $v->getElementsByTagName('UnionText5')->item(0)->nodeValue : '',
            'UnionCode5' => isset($v->getElementsByTagName('UnionCode5')->item(0)->nodeValue) ? $v->getElementsByTagName('UnionCode5')->item(0)->nodeValue : '',
            'UnionArchive5' => isset($v->getElementsByTagName('UnionArchive5')->item(0)->nodeValue) ? $v->getElementsByTagName('UnionArchive5')->item(0)->nodeValue : '',
            'UnionContact5' => isset($v->getElementsByTagName('UnionContact5')->item(0)->nodeValue) ? $v->getElementsByTagName('UnionContact5')->item(0)->nodeValue : '',
            'ProdImage' => isset($v->getElementsByTagName('ProdImage')->item(0)->nodeValue) ? $v->getElementsByTagName('ProdImage')->item(0)->nodeValue : '',
            'ProdImageText' => isset($v->getElementsByTagName('ProdImageText')->item(0)->nodeValue) ? $v->getElementsByTagName('ProdImageText')->item(0)->nodeValue : '',
            'ExtraData' => serialize($extra_data),
          ))
          ->execute();
  $roles = $v->getElementsByTagName('PerformerRole');
  foreach ($roles as $i => $role) {
    db_merge('calm_cache_performance_roles')
            ->key(array(
              'PerformanceID' => $v->getElementsByTagName('RecordID')->item(0)->nodeValue,
              'RoleCode' => trim($role->nodeValue),
            ))
            ->insertFields(array(
              'PerformanceID' => $v->getElementsByTagName('RecordID')->item(0)->nodeValue,
              'RoleCode' => trim($role->nodeValue),
              'created' => time(),
              'changed' => time(),
            ))
            ->updateFields(array(
              'changed' => time(),
            ))
            ->execute();
  }
  $roles = $v->getElementsByTagName('CreativeRole');
  foreach ($roles as $i => $role) {
    db_merge('calm_cache_performance_creatives')
            ->key(array(
              'PerformanceID' => $v->getElementsByTagName('RecordID')->item(0)->nodeValue,
              'RoleCode' => trim($role->nodeValue),
            ))
            ->insertFields(array(
              'PerformanceID' => $v->getElementsByTagName('RecordID')->item(0)->nodeValue,
              'RoleCode' => trim($role->nodeValue),
              'created' => time(),
              'changed' => time(),
            ))
            ->updateFields(array(
              'changed' => time(),
            ))
            ->execute();
  }
  if ($result == MergeQuery::STATUS_INSERT) {
    $counter++;
  }
  elseif ($result == MergeQuery::STATUS_UPDATE) {
    $updated++;
  }
}
