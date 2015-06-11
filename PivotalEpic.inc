<?php
/**
 * PivotalTracker Project Epic object
 * @author Schmuhl
 * @copyright 2015
 * @package PivotalTracker
 * @since 5/2015
 * @version 5/26/2015
 */



class PivotalEpic {

    public $epic_id;
    public $project_id;
    public $name;
    public $description;
    public $url;
    public $label;


    public static function getProjectEpics ( $project_id ) {
        if ( empty($project_id) || !is_numeric($project_id) ) return null;
        $epics = array();
        $result = Pivotal::callAPI("projects/$project_id/epics",'GET');
        foreach ( $result as $e ) {
            $epic = new PivotalEpic ();
            $epic->epic_id = $e->id;
            $epic->project_id = $e->project_id;
            $epic->name = $e->name;
            $epic->description = isset($e->description) ? $e->description : null;
            $epic->url = $e->url;
            $epic->label = $e->label->name;
            //echo "<pre>"; print_r($e); die();
            $epics[$epic->epic_id] = $epic;
        }
        //echo "<pre>"; print_r($result);
        return $epics;
    }
}