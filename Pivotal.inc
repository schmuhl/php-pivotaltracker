<?php
/**
 * Pivotal Object
 * @author Benjamin Schmuhl
 * @copyright 2009
 * @since 7/9/2009
 * @version 5.2
 * See: http://www.pivotaltracker.com/help/api
 * @todo Orginally written in 2009 for PivotalTracker API v3, now 5, so not all functionality will work
 */



require 'PivotalProject.inc';
require 'PivotalStory.inc';
require 'PivotalStoryTask.inc';
require 'PivotalIteration.inc';
require 'PivotalActivity.inc';
require 'PivotalEpic.inc';
require 'PivotalPerson.inc';



class Pivotal {

    public static $domain = 'https://www.pivotaltracker.com/';
	public static $cachePath = '/tmp';
	public static $cacheLength = 900; // 15 minutes
	public static $token;  // this needs to be set with the value from your Pivotal account or the calls won't work


    /**
     * Get the information about me from the API
     * @return object
     */
    public static function getMe () {
        $result = Pivotal::getResultJSON('me');
        return $result;
    }


	/**
	 * Get the activity from all projects
	 * @return array An array of PivotalActivity objects
	 */
	public static function getActivities ( $ignoreCache=false ) {
		$cacheFile = Pivotal::$cachePath.'projectActivity';

		// pull it from the cache or fresh?
		if ( file_exists($cacheFile) && filemtime($cacheFile) > time()-Pivotal::$cacheLength && !$ignoreCache ) {  // reasons to use the cache
			$contents = file_get_contents($cacheFile);
			$activities = unserialize($contents);
			return $activities;
		} else {  // okay, grab a fresh copy
			$activities = PivotalActivity::getActivities();
			$contents = serialize($activities);
			file_put_contents($cacheFile,$contents);
			return $activities;
		}
	}


    /**
     * Get the JSON from the PivotalTracker API
     * @param string $url
     * @param string $payload
     * @return object
     */
    public static function callAPI ( $url, $method='GET', $data=null ) {
        $method = strtoupper($method);
        if ( !in_array($method,array('GET','PUT','DELETE','POST')) ) {
            error_log("Method '$method' requested for call to $url with payload '".print_r($data,true)."'.");
            return false;
        }

        global $debug;

        // check to see if a valid cache file still exists, ie don't hit the API too often
        $cacheFile = Pivotal::$cachePath.DIRECTORY_SEPARATOR.'pt-'.md5($url);
        if ( file_exists($cacheFile) && filemtime($cacheFile) > time()-Pivotal::$cacheLength ) {  // if the file exists and isn't too old
            $json = file_get_contents($cacheFile);
        } else {
            // call the API
            $ch = curl_init();
            curl_setopt($ch, CURLOPT_URL,Pivotal::$domain.'services/v5/'.$url); // set url to post to
            curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
            curl_setopt($ch, CURLOPT_FAILONERROR, 0);
            curl_setopt($ch, CURLOPT_RETURNTRANSFER,1); // return into a variable
            $headers = array("X-TrackerToken: ".Pivotal::$token);
            curl_setopt($ch, CURLOPT_CUSTOMREQUEST, $method);
            // extra stuff for a POST
            if ( !empty($data) ) {
                curl_setopt($ch, CURLOPT_POSTFIELDS, $data);
                $headers []= 'Content-Type: application/json';
                $headers []= 'Content-Length: ' . strlen($data);
            }
            curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);

            if ( $debug ) $result = curl_exec($ch) or error_log("CURL: ".curl_error($ch)); // run the whole process
            else $result = curl_exec($ch); // run the whole process
            curl_close($ch);

            // cache the results for later
            if ( empty($data) ) @file_put_contents($cacheFile,$result);

            $result = json_decode($result);

            // make sure the response is kosher
            if ( isset($result->error) ) {
                error_log(Pivotal::$domain.'services/v5/'.$url."\n".print_r($data,true)."\n" . print_r($result, true));
                $json = null;
            } else $json = json_encode($result);
        }

        $object = json_decode($json);

        return $object;
    }


	/**
	 * Get the XML from the Pivotal API
	 * @param string $url
	 * @return string
     * @version 3
	 */
	public static function getResultXML ( $url ) {
		global $debug;
		
		$ch = curl_init();
		curl_setopt($ch, CURLOPT_URL,$url); // set url to post to
		curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
		curl_setopt($ch, CURLOPT_FAILONERROR, 0);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER,1); // return into a variable
		curl_setopt($ch, CURLOPT_HTTPHEADER, array("X-TrackerToken: ".Pivotal::$token));

		if ( $debug ) $result = curl_exec($ch) or addAlert("CURL: ".curl_error($ch)); // run the whole process
		else $result = curl_exec($ch); // run the whole process
		curl_close($ch);
		
		$xml = simplexml_load_string($result);
		return $xml;
	}


	/**
	 * Post to the Pivotal API
	 * @param string $url
	 * @param array
	 * @return string
     * @version 3
	 */
	public static function postXML ( $url, $xml, $method ) {
		global $debug;
		
		if ( $method != 'POST' && $method != 'PUT' ) $method = 'POST';
		
		$ch = curl_init();
		curl_setopt($ch, CURLOPT_URL,$url); // set url to post to
		curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
		
		if ( $debug ) curl_setopt($ch, CURLOPT_FAILONERROR, 0);
		else curl_setopt($ch, CURLOPT_FAILONERROR, 0);
		
		curl_setopt($ch, CURLOPT_RETURNTRANSFER,1); // return into a variable
		curl_setopt($ch, CURLOPT_HTTPHEADER, array( "$method ".$url." HTTP/1.0", "X-TrackerToken: ".Pivotal::$token, "Content-type: application/xml" ) );
		
		if ( 'POST' == $method ) {
			curl_setopt($ch, CURLOPT_POST, true);
			curl_setopt($ch, CURLOPT_POSTFIELDS, $xml );
		}
		if ( 'PUT' == $method ) {
			curl_setopt($ch, CURLOPT_PUT, true);
			$file = tmpfile();
			fwrite($file, $xml);
			fseek($file, 0);
			curl_setopt($ch, CURLOPT_INFILE, $file);
			curl_setopt($ch, CURLOPT_INFILESIZE, strlen($xml)); 
		}		
		
		if ( $debug ) $result = curl_exec($ch) or addAlert("CURL: ".curl_error($ch)); // run the whole process
		else $result = curl_exec($ch); // run the whole process
		
		curl_close($ch);
		if ( isset($file) ) fclose($file);

		$xml = simplexml_load_string($result);
		return $xml;		
	}
	
}

?>