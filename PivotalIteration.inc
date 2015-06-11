<?php

/**
 * Class PivotalIteration
 * @author Benjamin Schmuhl
 * @copyright 2009
 * @since 7/9/2009
 * @version 6/2/2015
 */
class PivotalIteration {

	public $number;
    public $project_id;
    public $team_strength;
	public $start;
	public $finish;
	public $stories;


	/**
	 * Copy from another similar object
	 * @param object $object
	 */
	public function copy ( $object ) {
		$this->stories = array();
		$vars = get_class_vars("PivotalIteration");
		foreach ( $vars as $var=>$value ) {  // loop through and set all of the variables
            if ( $var == 'stories' && is_array($value) ){  // stories array
                foreach ( $value as $s ) {
                    $story = new PivotalStory();
                    $story->copy($s);
                    $this->stories[$story->id] = $story;
                }
            } else if ( $var == 'start' || $var == 'finish' ) {  // dates
                $this->$var = ( !empty($object->$var) ) ? strtotime($object->$var) : null;
            } else if ( isset($object->$var) ) {  // everything else
                $this->$var = $object->$var;
            }
		}
	}
	
	
	/**
	 * Get the finish date in a consistent, human-readable format
	 * @return string
	 */
	public function getFinishDate () {
		if ( isset($this->finish) ) return date('n/d/Y',strtotime($this->finish));	
		return "?";
	}
	
	
	/**
	 * Get the start date in a consistent, human-readable format
	 * @return string
	 */
	public function getStartDate () {
		if ( isset($this->start) ) return date('n/d/Y',strtotime($this->start));	
		return "?";
	}
	
	
	/**
	 * Get the iterations from the API
	 * @param $projectID
	 * @return array
	 */
	public static function getIterations ( $projectID ) {
        if ( !is_numeric($projectID) ) return null;
        $result = Pivotal::callAPI("projects/$projectID/iterations",'GET');
        if ( is_array($result) ) {
            $iterations = array();
            foreach ( $result as $i ) {
                $iteration = new PivotalIteration();
                $iteration->copy($i);
                $iterations[$iteration->number] = $iteration;
            }
            //print_r($iterations); die();
            return $iterations;
        }
        error_log("Could not get project iterations: (".print_r($result,true).")");
        return false;
	}
	
	
	/**
	* Sort the iterations by finish date
	* @param PivotalIteration $a
	* @param PivotalIteration $b
	* @return int
	*/
	public static function sortByNewest ( $a, $b ) {
		if ( $a->finish > $b->finish ) return -1;
		if ( $a->finish < $b->finish ) return 1;
		return 0;
	}
}

?>