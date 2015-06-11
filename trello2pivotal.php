<?php
/**
 * Created by PhpStorm.
 * User: schmuhl
 * Date: 3/19/15
 * Time: 3:08 PM
 */


error_reporting(-1);
date_default_timezone_set("UTC");


require 'pivotal/Pivotal.inc';



Pivotal::$token = '90792c7a9b074c20b931e08d10833345';
Pivotal::$cacheLength = -1;
$project = new PivotalProject(1306954);
$project->getStories();
//echo "<pre>".print_r($project,true)."</pre>"; die();
//print_r(Pivotal::getMe()); die();

/*
Trello::$key = '5711f2206a0f843121169442dc2799a1';
Trello::$secret = '557b094e814bd8cf53912ed8b45531035310c0deab4c97921d0fd2dab6265ea3';
$board = new TrelloBoard ( '52d962e50f10dfde63da0f63' );  // development board
*/


$contents = file_get_contents($_GET['file']);
$trello = json_decode($contents);
//echo "<pre>".print_r($trello,true)."</pre>"; die();
//echo "<pre>".print_r(json_encode($trello, JSON_PRETTY_PRINT),true)."</pre>"; die();


//$trello->lists => new labels
//$trello->cards => issues
//$trello->labelNames => existing labels
//$trello->checklists => issue tasks


$storyCount = 0;
$taskCount = 0;
$storiesAdded = 0;

$skippedForDash = 0;
$skippedForDone = 0;
$skippedForBoard = 0;
$skippedForDuplicates = 0;

$newStories = array();
$failedStories = array();

//print_r($trello->cards);
foreach ( $trello->cards as $card ) {
    $storyCount ++;

    /* if ( $card->idBoard != '52d962e50f10dfde63da0f63' ) {
        $skippedForBoard ++;
        continue;
    } */

    if ( $card->closed == 1 ) {
        $skippedForDone ++;
        continue;
    }

    if ( strpos($card->name,"- ") === 0 ) {
        $skippedForDash ++;
        continue;
    }  // these ones have been imported already.

    $story = new PivotalStory();
    $story->project_id = $project->project_id;
    $story->name = $card->name;
    $story->description = $card->desc;
    if ( is_array($card->attachments) && count($card->attachments) > 0 ) {
        $story->description .= "\n\nSee attachments in Trello: ".$card->shortUrl;
    } else $story->description .= "\n\nFrom Trello: ".$card->shortUrl;
    $story->story_type = 'feature';
    $story->created_at = date('c',strtotime($card->dateLastActivity));
    //$story->owner = getTrelloMemberById($card->idMembers[0]);

    // turn the trello checklist into pivotal tasks
    foreach ( $card->idChecklists as $checklist ) {
        $list = getTrelloChecklistById($checklist);
        if (is_object($list)) {
            foreach ( $list->checkItems as $item ) {
                $task = new PivotalStoryTask ();
                $task->description = $item->name;
                $task->complete = ( $item->state == 'complete' );
                $story->tasks []= $task;
                $taskCount ++;
            }
        }
    }

    // add a label for the board
    //$story->labels [] = "Website";

    // turn the trello list into a pivotal label
    $list = getTrelloListById($card->idList);
    if ( is_object($list) ) {
        if ($list->name == "Done") {
            $skippedForDone ++;
            continue;
        }  // don't worry about these
        $story->labels [] = $list->name;
    }

    // turn the trello labels into a pivotal label
    foreach ( $card->labels as $label ) {
        if (is_object($label)) {
            $story->labels [] = $label->name;
        }
    }

    // skip stories that are already in Pivotal
    foreach ( $project->stories as $pstory ) {
       if ( $pstory->name == $story->name ) {
           $skippedForDuplicates ++;
           continue;
       }  // skip duplicates
    }

    // display this one
    if ( 1 == 2 ) {
        echo "$card->name<br/>";
        echo "<pre>".print_r($story,true)."</pre>";
        echo "<hr/><pre>".print_r($card,true)."</pre>";
        die();
    }

    $newStories []= $story;
    // save it
    if ( $story->save() ) {
        $storiesAdded ++;
    } else {
        $failedStories []= $story;
    }
}

echo "Looked at $storyCount stories with $taskCount tasks.";
echo "<br/>We tried to bring in ".count($newStories)." stories.";
echo "<br/> - There were $skippedForDash stories that were skipped for having a preceding dash.";
echo "<br/> - There were $skippedForBoard stories that were skipped because they weren't in the Develpment board.";
echo "<br/> - There were $skippedForDone stories that were skipped because they were already done.";
echo "<br/> - There were $skippedForDuplicates stories that were skipped because they are duplicates.";
echo "<br/>Only $storiesAdded stories were saved. The stories that could not be saved are listed below.";


echo "<pre style='margin: 10px; border: 1px solid gray; padding: 10px; height: 200px; overflow: auto;'>".print_r(json_encode($failedStories, JSON_PRETTY_PRINT),true)."</pre>"; die();






function getTrelloChecklistById ( $id ) {
    if ( empty($id) ) return false;
    global $trello;
    //echo "<pre>".print_r($trello->checklists,true)."</pre>"; die();
    foreach ( $trello->checklists as $checklist ) {
        if ( $checklist->id == $id ) return $checklist;
    }
    return false;
}

function getTrelloListById ( $id ) {
    if ( empty($id) ) return false;
    global $trello;
    foreach ( $trello->lists as $list ) {
        if ( $list->id == $id ) return $list;
    }
    return false;
}

/**
 * The members are an array provided by Trello. Get one of those members
 * @param $id
 * @return bool
 */
function getTrelloMemberById ( $id ) {
    if ( empty($id) ) return false;
    global $trello;
    foreach ( $trello->members as $member ) {
        if ( $member->id == $id ) return $member;
    }
    return false;
}



?>