<?php

/**
<?xml version="1.0" encoding="UTF-8"?>
    <activities type="array">
      <activity>
        <id type="integer">1031</id>
        <project>Sample Project</project>
        <story>More power to shields</story>
        <description>James Kirk accepted &quot;More power to shields&quot;</description>
        <author>James Kirk </author>
        <when>06/01/2009 08:22 AM</when>
      </activity>
      <activity>
        <id type="integer">1030</id>
        <project>Another Sample Project</project>
        <story>Warp speed</story>w
        <description>Montgomery Scott rejected &quot;Warp speend&quot; with comments: &quot;Warp speed isn't working&quot;</description>
        <author>Montgomery Scott</author>
        <when>06/01/2009 12:22 AM</when>
      </activity>
    </activities>

*/
class PivotalActivity {

	public $type;
	public $project;
	public $story;
	public $description;
	public $author;
	public $when;


	public function __construct () {}


	public function copy ( $object ) {
		$vars = get_class_vars("PivotalActivity");
		foreach ( $vars as $var=>$value ) {  // loop through and set all of the variables
			if ( isset($object->$var) ) $this->$var = (string)$object->$var;
		}
	}


	public static function getActivities () {
		$url = "http://www.pivotaltracker.com/services/v2/activities";  // get all stories
		$xml = Pivotal::getResultXML($url);

		$activities = array();
		if ( isset($xml->activity) ) {
			foreach ( $xml->activity as $temp ) {
				$activity = new PivotalActivity ();
				$activity->copy($temp);
				$activities []= $activity;
			}
		} else die('Malformed response');
		return $activities;
	}


	/**
	 * Sort the PivotalActivity objects by date then by story
	 * @param $a PivotalStory
	 * @param $b PivotalStory
	 * @return int
	 * @todo The sorting of dates may not be right...
	 */
	public static function sortByDateThenStory ( $a, $b ) {
		$aDate = date("Y-m-d",strtotime($a->when));
		$bDate = date("Y-m-d",strtotime($b->when));
		if ( $aDate > $bDate ) return -1;
		if ( $aDate < $bDate ) return 1;
		if ( $a->story > $b->story ) return 1;
		if ( $b->story < $b->story ) return -1;
        return 0;
	}

}

?>