<?php
/**
 * Created by PhpStorm.
 * User: schmuhl
 * Date: 4/10/15
 * Time: 9:12 AM
 */


error_reporting(-1);
date_default_timezone_set("UTC");


require 'pivotal/Pivotal.inc';



// what are we doing?
$hasLabel = ( isset($_GET['hasLabel']) ) ? $_GET['hasLabel'] : null;
$hasType = ( isset($_GET['hasType']) ) ? $_GET['hasType'] : null;

$setType = ( isset($_GET['setType']) ) ? $_GET['setType'] : null;
$addLabel = ( isset($_GET['addLabel']) ) ? $_GET['addLabel'] : null;

$excludedStates = array( 'accepted' );



Pivotal::$token = '90792c7a9b074c20b931e08d10833345';
Pivotal::$cacheLength = -1;
$project = new PivotalProject(1306954);
$project->getStories();
//echo "<pre>".print_r($project,true)."</pre>"; die();

?>

<h1>Pivotal Batcher</h1>

<?php
if ( empty($hasLabel) && empty($hasType) ) {  // nothing to do
    ?>
    <p>
        This tool allows you to select certain stories to be changed.
    </p>
    <form method="get" action="pivotalBatcher.php">
        <div class="field">
            <label>Has label:</label>
            <input type="text" name="hasLabel" />
        </div>
        <div class="field">
            <label>Has type:</label>
            <input type="text" name="hasType" />
        </div>
        <hr/>
        <div class="field">
            <label>Add label:</label>
            <input type="text" name="addLabel" />
        </div>
        <div class="field">
            <label>Set type:</label>
            <input type="text" name="setType" />
        </div>
        <div class="field">
            <label>Just test:</label>
            <input type="checkbox" name="test" checked />
        </div>
        <input type="submit" value="Go" />
    </form>
    ?>
<?php
} else {  // --------------------- run it
    ?>


<p>
    Looking through <?php echo count($project->stories); ?> stories for those with
    <?php if ( !empty($hasLabel) ) echo " a label of '$hasLabel', "; ?>
    <?php if ( !empty($hasType) ) echo " a type of '$hasType', "; ?>
    so that we can
    <?php if ( !empty($addLabel) ) echo " add a label of '$addLabel', "; ?>
    <?php if ( !empty($setType) ) echo " set the type to '$setType', "; ?>
    and that's all.
</p>


    <?php
    foreach ( $project->stories as $story ) {
        //if ( $story->id == 91056540 ) { echo "<pre>".print_r($story,true)."</pre>"; die(); }

        if ( ( empty($hasLabel) || in_array($hasLabel,$story->getLabels()) )
            && ( empty($hasType) || $story->story_type == $hasType )
            && ( !in_array($story->current_state,$excludedStates) )
        )
        {
            echo "<br/>Updating story #$story->id...";
            $updateStory = new PivotalStory ();
            $updateStory->project_id = $story->project_id;
            $updateStory->id = $story->id;

            if ( !empty($setType) ) {
                if ( $story->story_type == $setType ) continue;
                $updateStory->story_type = $setType;

            } // change the type
            // add label
            if ( !$updateStory->save() ) {
                echo "<pre>" . print_r($updateStory, true) . "</pre>";
                die();
            }
        }

    }

}

?>