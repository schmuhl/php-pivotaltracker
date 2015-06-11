<?php
/**
 * Pivotal Story Object
 * @author Benjamin Schmuhl
 * @copyright 2009
 * @since 7/9/2009
 * @version 12/14/2009
 * See: http://www.pivotaltracker.com/help/api
 *
[id] => 918310
[story_type] => feature
[url] => http://www.pivotaltracker.com/story/show/918310
[estimate] => 1
[current_state] => accepted
[description] => SimpleXMLElement Object
    (
    )

[name] => Add credit score as a factor
[requested_by] => Benjamin Schmuhl
[owned_by] => Benjamin Schmuhl
[created_at] => 2009/07/02 18:00:00 MDT
[accepted_at] => 2009/07/05 18:00:00 MDT
[iteration] => SimpleXMLElement Object
    (
        [number] => 1
        [start] => 2009/06/29 00:00:00 MDT
        [finish] => 2009/07/06 00:00:00 MDT
    )
 */
class PivotalStory {

	public $id;
    public $project_id;
	public $story_type;
	public $url;
	public $estimate;
	public $current_state;
	public $description;
	public $name;
	public $created_at;
	public $accepted_at;
	public $labels;
    public $owner_ids;
	//public $iteration;
    //public $requested_by;
    //public $owned_by;

    public $tasks;


    public function __construct () {
        $this->current_state = 'unscheduled';
        $this->labels = array();
    }



	/**
	 * Copy from another similar object
	 * @param object $object
	 */
	public function copy ( $object ) {
		$vars = get_class_vars("PivotalStory");
		foreach ( $vars as $var=>$value ) {  // loop through and set all of the variables
			if ( isset($object->$var) ) $this->$var = $object->$var;
		}

		if ( isset($object->iteration) ) {  // handle the iteration
			$this->iteration = new PivotalIteration ();
			$this->iteration->copy($object->iteration);
		}

	}
	

	/**
	* Get the created date
	* @return string
	*/
	public function getCreated () {
		if ( isset($this->created_at) && !empty($this->created_at) ) return date("n/d/Y",strtotime($this->created_at));
		return null;
	}


	public function getDate () {
		if ( isset($this->accepted_at) && !empty($this->accepted_at) ) return date("n/d/Y",strtotime($this->accepted_at));
		if ( isset($this->iteration->finish) ) return date('n/d/Y',strtotime($this->iteration->finish));
        return null;
	}


    /**
     * Get all of this story's labels as an array of strings
     * @return array of strings
     */
    public function getLabels () {
        $labels = array();
        foreach ( $this->labels as $label ) $labels []= $label->name;
        return $labels;
    }


    /**
     * Add a label to the story
     * @param string $label
     * @return bool Success
     */
    public function addLabel ( $label ) {
        if ( empty($label) ) return false;
        if ( !is_array($this->labels) ) $this->labels = array();
        $this->labels []= $label;
        return true;
    }


    /**
     * Check to see if this function is really valid
     * @return bool
     */
    public function isValid () {
        if ( empty($this->project_id) ) return false;
        if ( empty($this->name) ) return false;
        if ( !in_array($this->current_state,PivotalStory::getStates()) ) return false;
        if ( !in_array($this->story_type,PivotalStory::getStoryTypes()) ) return false;
        if ( empty($this->created_at) ) return false;  // the date needs to be in ISO 8601 format date('c'
        return true;
    }


    /**
     * Save this object as a PivotalTracker story
     */
    public function save () {
        if ( !$this->isValid() ) return false;

        // need to make two separate API calls here
        $tasks = $this->tasks;
        unset($this->tasks);

        if ( empty($this->id) ) { // add this story, it's new!
            // save the story
            $result = Pivotal::callAPI("projects/$this->project_id/stories", 'POST', json_encode($this));
            if (isset($result->error)) {
                echo "<pre>" . print_r($result, true) . "</pre>";
                return false;
            } else {
                $this->id = $result->id;
            }

        } else {  // Update this story
            $result = Pivotal::callAPI("projects/$this->project_id/stories/$this->id",'PUT',json_encode($this));
            if (isset($result->error)) {
                echo "<pre>" . print_r($result, true) . "</pre>";
                return false;
            }


        }

        // update the tasks
        $this->tasks = $tasks;
        // save the tasks
        if (isset($tasks) && is_array($tasks) && count($tasks) > 0) {
            //echo "services/v5/projects/$this->project_id/stories/$this->id/tasks";
            //print_r(json_encode($tasks));
            foreach ($tasks as $task) {
                unset($task->position);
                $result = Pivotal::callAPI("projects/$this->project_id/stories/$this->id/tasks", 'POST', json_encode($task));
                if (isset($result->error)) {
                    echo "<pre>" . print_r($result, true) . "</pre>";
                    return false; // the story saved, but not the tasks
                }
            }
        }

        return true;
    }


    /**
     * Delete this through the API
     * @return boolean Success
     */
    public function delete () {
        if ( empty($this->project_id) || empty($this->id) ) return false;
        $result = Pivotal::getResultJSON("services/v5/projects/$this->project_id/stories/$this->id", -1);
        if ( isset($result->error) ) {
            echo "<pre>" . print_r($result, true) . "</pre>";
            return false;
        }
        return true;
    }


    /**
     * Add a comment to the story
     * @param string $text The comment you want added
     * @param string $person_id The id of the person who is to post the comment
     * @return boolean Success
     */
    public function addComment ( $text, $person_id=null ) {
        if ( empty($this->project_id) || empty($text) ) return false;
        $comment = new stdClass();
        $comment->text = $text;
        $comment->person_id = !empty($person_id) ? $person_id : $this->requested_by_id;
        $result = Pivotal::callAPI("projects/$this->project_id/stories/$this->id/comments", 'POST', json_encode($comment));
        if ( isset($result->error) ) {
            echo "<pre>" . print_r($result, true) . "</pre>";
            return false;
        }
        return true;
    }


    public function isCompleted () {
        if ( $this->current_state == 'accepted' ) return true;
        if ( in_array($this->current_state,array('accepted','delivered')) ) return true;
        return false;
    }



	public static function getStates () {
		return array ( 'started','finished','delivered','accepted','rejected','planned','unstarted','unscheduled' );
	}


	public static function getStoryTypes () {
		return array ( 'feature','bug','chore','release' );
	}


	/**
	 * Get the stories from the API
	 * @param $projectID
	 * @return array
     * @deprecated
	 */
	public static function getStories ( $projectID, $user=null ) {
		$url = "http://www.pivotaltracker.com/services/v2/projects/$projectID/stories";  // get all stories
		if ( isset($user) ) $url .= "?filter=mywork%3A$user";  // ?filter=mywork:Spock
		$xml = Pivotal::getResultXML($url);
		$stories = array();
		if ( isset($xml->story) ) {
			foreach ( $xml->story as $temp ) {
				$story = new PivotalStory ();
				$story->copy($temp);
				$story->project_id = $projectID;
				$stories []= $story;
			}
		} else addAlert('Malformed response on '.$url);
		return $stories;
	}


	/**
	 * Sort the Stories by owner
	 * @param PivotalStory $a
	 * @param PivotalStory $b
	 * @return int
	 */
	public static function sortByOwner ( $a, $b ) {
		if ( $a->accepted_at > $b->accepted_at ) return 1;
		if ( $a->accepted_at < $b->accepted_at ) return -1;

		return 0;
	}


	/**
	 * Sort the stories most recent to the oldest
	 * @param PivotalStory $a
	 * @param PivotalStory $b
	 * @return int
	 */
	public static function sortByNewest ( $a, $b ) {
		if ( $a->accepted_at == $b->accepted_at ) return 0;		
		if ( empty($a->accepted_at) ) return -1;
		if ( empty($b->accepted_at) ) return 1;
		if ( $a->accepted_at < $b->accepted_at ) return 1;
		if ( $a->accepted_at > $b->accepted_at ) return -1;
		return 0;
	}


	/**
	 * Sort the stories oldest to most recent
	 * @param PivotalStory $a
	 * @param PivotalStory $b
	 * @return int
	 */
	public static function sortByOldest ( $a, $b ) {
		if ( $b->created_at == $a->created_at ) return 0;		
		if ( empty($b->created_at) ) return -1;
		if ( empty($a->created_at) ) return 1;
		if ( $b->created_at < $a->created_at ) return 1;
		if ( $b->created_at > $a->created_at ) return -1;
		return 0;
	}
	
	
	/**
	 * Sort the stories for the history, most recent to oldest
	 * @param PivotalStory $a
	 * @param PivotalStory $b
	 * @return int
	 */
	public static function sortHistory ( $a, $b ) {
		if ( !isset($a->owned_by) || !isset($b->owned_by) ) {  // if there is no owner
			if ( !isset($a->owned_by) ) return 1;
			if ( !isset($b->owned_by) ) return -1;
		}
		if ( isset($a->owned_by) && !empty($a->owned_by) && isset($b->owned_by) && !empty($b->owned_by) ) {  // if there is a missing owner, take 2
			if ( $a->owned_by > $b->owned_by ) return 1;
			if ( $a->owned_by < $b->owned_by ) return -1;
		}

		return 0;
	}

}




?>