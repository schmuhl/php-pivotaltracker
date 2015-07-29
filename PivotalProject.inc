<?php
/**
 * PivotalTracker Project object
 * @author Schmuhl
 * @copyright 2009
 * @package PivotalTracker
 * @since 7/2009
 * @version 5/26/2015
 */
class PivotalProject {

	public $project_id;
    public $name;
	public $storyCount;
	public $acceptedStoryCount;
    public $stories;

    public $epics;
    public $iterations;
    public $people;


	public function __construct ( $project_id=null ) {
        $this->stories = null;
        $this->epics = null;
        $this->people = null;
        $this->iterations = null;
        if ( empty($project_id) || !is_numeric($project_id) ) return;
        $result = Pivotal::callAPI("projects/$project_id",'GET');
        //print_r($result); //die();
        $this->project_id = isset($result->id) ? $result->id : null;
        $this->name = isset($result->name) ? $result->name : null;
	}


    /**
     * Do a quick check to see if this project seems valid
     * @return bool
     */
    public function isValid () {
        if ( isset($this->project_id) && !empty($this->name) ) return true;
        return false;
    }


    /**
     * Add a story to this project
     * @param PivotalStory $story
     */
    public function addStory ( $story ) {
        $json = json_encode($story);
        Pivotal::getResultJSON("/projects/$this->project_id/stories");

    }
	
	
	/**
	* Save this story back into Pivotal
     * @param $story PivotalStory
     * @deprecated Version 3
     * @version 3
	* @return boolean
	*/
	public function saveStory ( $story ) {
		// curl -H "X-TrackerToken: TOKEN" -H "Content-type: application/xml" \
		// -d "<story><story_type>feature</story_type><name>Fire torpedoes</name><requested_by>James Kirk</requested_by></story>" \
		// -X POST http://www.pivotaltracker.com/services/v2/projects/PROJECT_ID/stories
		
		// curl -H "X-TrackerToken: TOKEN" -H "Content-type: application/xml" \
		// -d "<story><estimate>2</estimate><labels>tribbles, phaser</labels></story>" \
		// -X PUT http://www.pivotaltracker.com/services/v2/projects/PROJECT_ID/stories/STORY_ID
			
		if ( !empty($story->id) ) $url = "http://www.pivotaltracker.com/services/v3/projects/$this->project_id/stories/$story->id";
		else $url = "http://www.pivotaltracker.com/services/v3/projects/$this->project_id/stories";
		$xml = "<story><name>$story->name</name><story_type>$story->story_type</story_type><description>$story->description</description><estimate>$story->estimate</estimate><requested_by>$story->requested_by</requested_by><owned_by>$story->owned_by</owned_by></story>";

		$xml = Pivotal::postXML($url,$xml,'PUT');
		/*
		Object (
			[@attributes] => Array (
				[success] => false
        	)
    		[0] => 500 error
		)
		*/
		@$attributes = $xml->attributes();
							
		if ( isset($xml->error) || ( isset($attributes->success) && 'false' == $attributes->success ) ) {
			/** @todo Shouldn't call a function like this within an object */
			//addAlert("Could not save story #$story->id: $xml");
			return false;
		}
		return true;	
	}
	

	/**
	 * Get the stories for a project, using caching
	 * @param boolean $ignoreCache Ignore the cache and pull a fresh copy?
	 * @return array An array of PivotalStory objects
	 */
	public function getStories ( $ignoreCache=false, $user=null, $justActiveStories=false ) {
        if ( is_array($this->stories) ) return $this->stories;
        $result = Pivotal::callAPI("projects/$this->project_id/stories?limit=10000",'GET');
        //die(print_r($result,true));
        $stories = array();
        foreach ( $result as $t ) {
            $story = new PivotalStory();
            $story->copy($t);
            $stories[$story->id] = $story;
        }
        $this->stories = $stories;
        return $this->stories;
	}
	

	/**
	 * Get the iterations and stories for a project, using caching
	 * @param boolean $ignoreCache Ignore the cache and pull a fresh copy?
	 * @return array An array of PivotalStory objects
	 */
	public function getIterations ( $ignoreCache=false ) {
        if ( is_array($this->iterations) ) return $this->iterations;
        $this->iterations = PivotalIteration::getIterations($this->project_id);
        return $this->iterations;
	}


    /**
     * Get the current iteration (known by current time)
     * @return PivotalIteration
     */
    public function getCurrentIteration () {
        $this->getIterations();
        if ( is_array($this->iterations) ) {
            foreach ($this->iterations as $iteration) {
                //echo "<br/>Looking at iteration $iteration->project_id #$iteration->number from ".date('n/d/Y g:ia',$iteration->start)." to ".date('n/d/Y g:ia',$iteration->finish);
                if (time() > $iteration->start && time() < $iteration->finish) {
                    return $iteration;
                }
            }
        }
        die("<br/>$this->name has no current iteration!");
        return null;
    }


    /**
     * Get the users for a project
     * @return array An array of PivotalUser objects
     */
    public function getPeople () {
        if ( is_array($this->people) ) return $this->people;
        $this->people = PivotalPerson::getPeople($this->project_id);
        return $this->people;
    }


    /**
     * Get all of the epics for this project
     * @return mixed
     */
    public function getEpics () {
        if ( is_array($this->epics) ) return $this->epics();
        $this->epics = PivotalEpic::getProjectEpics($this->project_id);
        return $this->epics;
    }


	/**
	 * @todo Make this work
	 */
	public function getCurrent ( $projectID ) {

		$url = "https://www.pivotaltracker.com/services/v2/projects/$projectID/iterations/current";
		$xml = getResultXML($url);

		$iteration = $xml->iteration->number;
		$started = $xml->iteration->start;

		$p_cnt = count($xml->iteration->stories->story);
		for($i = 0; $i < $p_cnt; $i++) {
			$story = $xml->iteration->stories->story[$i];
			print_r($story->name." ".$story->labels."<hr>");
		}
	}


}


?>