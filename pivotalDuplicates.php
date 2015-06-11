<?php
/**
 * Created by PhpStorm.
 * User: schmuhl
 * Date: 4/9/15
 * Time: 10:03 PM
 */



error_reporting(-1);
date_default_timezone_set("UTC");


require_once 'pivotal/Pivotal.inc';



Pivotal::$token = '90792c7a9b074c20b931e08d10833345';
Pivotal::$cacheLength = -1;
$project = new PivotalProject(1306954);
$project->getStories();

$duplicateCount = 0;
$deleted = 0;


foreach ( $project->stories as $story ) {
    if ( $story->requested_by_id != 1594994 ) continue;  // only mine
    foreach ( $project->stories as $story2 ) {
        if ( $story->id == $story2->id ) continue;  // skip the same

        if ( $story->name == $story2->name ) {

            if ( count($story->labels) > count($story2->labels)
                || count($story->tasks) > count($story2->tasks)
                || strtotime($story->updated_at) > strtotime($story2->updated_at) )
            {
                if ( $story2->delete() ) {
                    $deleted ++;
                    continue;
                }
            }


            $duplicateCount ++;
            ?>
            <pre style="float: left; margin: 5px; border: 1px solid gray; padding: 5px; width: 48%; overflow: hidden;"><?php echo print_r($story,true); ?></pre>
            <pre style="float: right; margin: 5px; border: 1px solid gray; padding: 5px; width: 48%; overflow: hidden;"><?php echo print_r($story2,true); ?></pre>
            <hr style="clear: both;" />
            <?php

        }
    }
}

?>

<script>
    if ( confirm("There are <?php echo count($project->stories); ?> stories and <?php echo $duplicateCount; ?> are duplicates and <?php echo $deleted; ?> were deleted.") ) {
        location.href = 'index.php';
    }
</script>

