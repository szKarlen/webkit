<?php

require('../include/json-header.php');

function main($path) {
    $db = new Database;
    if (!$db->connect())
        exit_with_error('DatabaseConnectionFailure');

    if (count($path) > 1)
        exit_with_error('InvalidRequest');

    if (count($path) > 0 && $path[0]) {
        $task_id = intval($path[0]);
        $task = $db->select_first_row('analysis_tasks', 'task', array('id' => $task_id));
        if (!$task)
            exit_with_error('TaskNotFound', array('id' => $task_id));
        $tasks = array($task);
    } else {
        $metric_id = array_get($_GET, 'metric');
        $platform_id = array_get($_GET, 'platform');
        if (!!$metric_id != !!$platform_id)
            exit_with_error('InvalidArguments', array('metricId' => $metric_id, 'platformId' => $platform_id));

        if ($metric_id)
            $tasks = $db->select_rows('analysis_tasks', 'task', array('platform' => $platform_id, 'metric' => $metric_id));
        else {
            // FIXME: Limit the number of tasks we fetch.
            $tasks = array_reverse($db->fetch_table('analysis_tasks', 'task_created_at'));
        }

        if (!is_array($tasks))
            exit_with_error('FailedToFetchTasks');
    }

    $tasks = array_map("format_task", $tasks);
    $bugs = fetch_and_push_bugs_to_tasks($db, $tasks);

    exit_with_success(array('analysisTasks' => $tasks, 'bugs' => $bugs));
}

function fetch_and_push_bugs_to_tasks($db, &$tasks) {
    $task_ids = array();
    $task_by_id = array();
    foreach ($tasks as &$task) {
        array_push($task_ids, $task['id']);
        $task_by_id[$task['id']] = &$task;
    }

    $bugs = $db->query_and_fetch_all('SELECT bug_id AS "id", bug_task AS "task", bug_tracker AS "bugTracker", bug_number AS "number"
        FROM bugs WHERE bug_task = ANY ($1)', array('{' . implode(', ', $task_ids) . '}'));
    if (!is_array($bugs))
        exit_with_error('FailedToFetchBugs');

    foreach ($bugs as $bug) {
        $associated_task = &$task_by_id[$bug['task']];
        array_push($associated_task['bugs'], $bug['id']);
    }

    return $bugs;
}

date_default_timezone_set('UTC');
function format_task($task_row) {
    return array(
        'id' => $task_row['task_id'],
        'name' => $task_row['task_name'],
        'author' => $task_row['task_author'],
        'createdAt' => strtotime($task_row['task_created_at']) * 1000,
        'platform' => $task_row['task_platform'],
        'metric' => $task_row['task_metric'],
        'startRun' => $task_row['task_start_run'],
        'endRun' => $task_row['task_end_run'],
        'bugs' => array(),
    );
}

main(array_key_exists('PATH_INFO', $_SERVER) ? explode('/', trim($_SERVER['PATH_INFO'], '/')) : array());

?>
