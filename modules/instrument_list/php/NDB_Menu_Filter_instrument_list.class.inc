<?php
/**
 * Instrument_List
 *
 * PHP Version 5
 *
 * @category Main
 * @package  Instrument_List
 * @author   Loris Team <loris.mni@bic.mni.mcgill.ca>
 * @license  http://www.gnu.org/licenses/gpl-3.0.txt GPLv3
 * @link     https://github.com/aces/Loris
 */
/**
 * The timepoint details menu
 *
 * @package behavioural
 */
require_once __DIR__ ."/Instrument_List_ControlPanel.class.inc";
/**
 * Instrument_List
 *
 * PHP Version 5
 *
 * @category Main
 * @package  Instrument_List
 * @author   Loris Team <loris.mni@bic.mni.mcgill.ca>
 * @license  http://www.gnu.org/licenses/gpl-3.0.txt GPLv3
 * @link     https://github.com/aces/Loris
 */
class NDB_Menu_Filter_Instrument_List extends NDB_Menu_Filter
{
    /**
    * Checking permissions
    *
    * @return bool
    */
    function _hasAccess()
    {
        // create user object
        $user =& User::singleton();

        $timePoint =& TimePoint::singleton($_REQUEST['sessionID']);
        $candID    = $timePoint->getCandID();

        $candidate =& Candidate::singleton($candID);

        // check user permissions
        return ($user->hasPermission('access_all_profiles')
                || (in_array(
                    $candidate->getData('CenterID'),
                    $user->getData('CenterIDs')
                )
                   )
                || (in_array(
                    $timepoint->getData('CenterID'),
                    $user->getData('CenterIDs')
                )
                   )
        );

    }
    /**
    * GetControlPanel function
    *
    * @return string
    */
    function getControlPanel()
    {
        $controlPanel = new Instrument_List_ControlPanel(
            $_REQUEST['sessionID']
        );
        // save possible changes from the control panel...
        $success = $controlPanel->save();

        // display the control panel
        $html = $controlPanel->display();
        // I don't know why this is here, but it
        // was in main.php, so moved it to be safe.
        $timePoint =& TimePoint::singleton($_REQUEST['sessionID']);
        $timePoint->select($_REQUEST['sessionID']);
        return $html;
    }
    /**
    * Setup function
    *
    * @return void
    */
    function setup()
    {
        // set template data
        $this->tpl_data['candID']    = $_REQUEST['candID'];
        $this->tpl_data['sessionID'] = $_REQUEST['sessionID'];

        $this->_setupPage(null, null, null, null, 'filter');
        // get behavioral battery for this visit (time point)
        $battery = new NDB_BVL_Battery;
        $success = $battery->selectBattery($_REQUEST['sessionID']);

        $this->tpl_data['stage']        = Utility::getStageUsingCandID(
            $this->tpl_data['candID']
        );
        $this->tpl_data['subprojectID'] = Utility::getSubprojectIDUsingCandID(
            $this->tpl_data['candID']
        );

        // get the list of instruments
        $listOfInstruments = $this->getBatteryInstrumentList(
            $this->tpl_data['stage'],
            $this->tpl_data['subprojectID']
        );

        // display list of instruments
        if (!empty($listOfInstruments)) {
            $user     =& User::singleton();
            $username = $user->getData('UserID');

            $feedback_select_inactive = null;
            if ($user->hasPermission('bvl_feedback')) {
                $feedback_select_inactive = 'Y';
            }

            $x            = -1;
            $prevSubgroup = null;
            foreach ($listOfInstruments as $instrument) {
                // print the sub group header row
                if ($instrument['Sub_group'] != $prevSubgroup) {
                    $x++;
                    $i = 0;
                    $this->tpl_data['instrument_groups'][$x]['title']
                        = $instrument['Sub_group'];
                }
                $prevSubgroup = $instrument['Sub_group'];

                // make an instrument status object
                $status  = new NDB_BVL_InstrumentStatus;
                $success = $status->select($instrument['CommentID']);

                $ddeStatus = new NDB_BVL_InstrumentStatus;
                $success   = $ddeStatus->select($instrument['DDECommentID']);

                $Ins = "instruments";

                $ins = $instrument['Full_name'];
                $this->tpl_data[$Ins][$x][$i]['fullName'] = $ins;

                $ins = $status->getDataEntryStatus();
                $this->tpl_data[$Ins][$x][$i]['dataEntryStatus'] = $ins;

                $ins = $status->getAdministrationStatus();
                $adm = "administrationStatus";
                $this->tpl_data[$Ins][$x][$i][$adm] = $ins;

                $ins = $instrument['Test_name'];
                $this->tpl_data[$Ins][$x][$i]['testName'] = $ins;

                $ins = $instrument['CommentID'];
                $this->tpl_data[$Ins][$x][$i]['commentID'] = $ins;

                $ins = NDB_BVL_Battery::isDoubleDataEntryEnabledForInstrument(
                    $instrument['Test_name']
                );
                $this->tpl_data[$Ins][$x][$i]['isDdeEnabled'] = $ins;

                $ins = $instrument['DDECommentID'];
                $this->tpl_data[$Ins][$x][$i]['ddeCommentID'] = $ins;

                $ins = $ddeStatus->getDataEntryStatus();
                $this->tpl_data[$Ins][$x][$i]['ddeDataEntryStatus'] =  $ins;

                $ins = $instrument['isDirectEntry'];
                $this->tpl_data[$Ins][$x][$i]['isDirectEntry'] = $ins;

                $ins =  $instrument['instrument_order'];
                $this->tpl_data[$Ins][$x][$i]['instrumentOrder'] = $ins;

                // create feedback object for the time point
                $feedback = NDB_BVL_Feedback::singleton(
                    $username,
                    null,
                    null,
                    $instrument['CommentID']
                );

                $feedback_status = $feedback->getMaxThreadStatus(
                    $feedback_select_inactive
                );

                $feedback_count = $feedback->getThreadCount();

                $this->tpl_data[$Ins][$x][$i]['feedbackCount']
                    =       (empty($feedback_count))
                             ? $feedback_status : $feedback_count;
                if (!empty($feedback_status)) {
                    $this->tpl_data[$Ins][$x][$i]['feedbackStatus']
                        = $feedback_status;
                    if ($feedback_count > 0) {
                        $this->tpl_data[$Ins][$x][$i]['feedbackColor']
                            = $feedback->getThreadColor($feedback_status);
                    }
                } else {
                    $this->tpl_data[$Ins][$x][$i]['feedbackStatus'] = "-";
                }

                $i++;
            }
        }

        $timePoint =& TimePoint::singleton($_REQUEST['sessionID']);
        $candID    = $timePoint->getCandID();

        $candidate =& Candidate::singleton($candID);

        $this->tpl_data['display']
            = array_merge($candidate->getData(), $timePoint->getData());
    }
    /**
     * Used by the NDB_caller class when loading a page.
     * Call the display function of the appropriate modules feedback panel.
     *
     * @param string $candID    the value of candID
     * @param string $sessionID the value of sessionID
     *
     * @return void
     */
    function getFeedbackPanel($candID, $sessionID)
    {
        $feedbackPanel = new BVL_Feedback_Panel($candID, $sessionID);
        $html          =  $feedbackPanel->display();
        return $html;
    }

    /**
     * Gets an associative array of instruments which are members of the current
     * battery for instrument list module
     *
     * @param string  $stage        Either 'visit' or 'screening' - determines
     *                              whether to register only screening instruments
     *                              or visit instruments
     * @param integer $SubprojectID The SubprojectID of that we want the battery for.
     *
     * @return array an associative array containing Test_name,
     *         Full_name, Sub_group, CommentID
     */
    function getBatteryInstrumentList($stage=null, $SubprojectID=null)
    {
        $DB = Database::singleton();

        // assert that a battery has already been selected
        if (is_null($_REQUEST['sessionID'])) {
            throw new Exception("No battery selected");
        }

        // craft the select query
        $query = "SELECT DISTINCT f.Test_name,
                    t.Full_name,
                    f.CommentID,
                    CONCAT('DDE_', f.CommentID) AS DDECommentID,
                    ts.Subgroup_name as Sub_group,
                    ts.group_order as Subgroup_order,
                    t.isDirectEntry";
        if (!is_null($stage)) {
            $query .= ", b.instr_order as instrument_order";
        }
        $query  .= " FROM flag f
            	JOIN test_names t ON (f.Test_name=t.Test_name)
            	JOIN test_subgroups ts ON (ts.ID = t.Sub_group)
            	LEFT JOIN session s ON (s.ID=f.SessionID) ";
        $qparams = array('SID' => $_REQUEST['sessionID']);

        if (!is_null($stage)) {
            $query .= "
            	LEFT JOIN test_battery b 
       	        ON ((t.Test_name=b.Test_name OR b.Test_name IS NULL) 
      	        AND (s.SubprojectID=b.SubprojectID OR b.SubprojectID IS NULL) 
       	        AND (s.Visit_label=b.Visit_label OR b.Visit_label IS NULL) 
       	        AND (s.CenterID=b.CenterID OR b.CenterID IS NULL)) ";
        }
        $query .= " WHERE f.SessionID=:SID
            AND LEFT(f.CommentID, 3) != 'DDE'";

        if (Utility::columnsHasNull('test_subgroups', 'group_order')) {
            $query .= " ORDER BY Subgroup_name";
        } else {
            $query .= " ORDER BY Subgroup_order";
        }
        if (!is_null($stage)) {
            $query .= ", instrument_order";
        } else {
            $query .= ", Full_name";
        }
        //print_r($query);
        // get the list of instruments
        $rows = $DB->pselect($query, $qparams);

        // return all the data selected
        return $rows;
    } // end getBatteryInstrumentList()

    /**
     * Include the column formatter required to display the feedback link colours
     * in the candidate_list menu
     *
     * @return array of javascript to be inserted
     */
    function getJSDependencies()
    {
        $factory = NDB_Factory::singleton();
        $baseURL = $factory->settings()->getBaseURL();
        $deps    = parent::getJSDependencies();
        return array_merge(
            $deps,
            array(
             $baseURL . "/instrument_list/js/instrument_list_helper.js",
            )
        );
    }
}
?>
