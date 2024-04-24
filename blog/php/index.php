<?php
$files = glob('./blog/php/txt/*.txt');
foreach ($files as $file) {
    $filename = basename($file, '.txt');
    list($date, $title) = explode('_', $filename, 2);
    $content = file_get_contents($file); 

    echo "<h2>$title</h2>";
    echo "<p><i>Date: " . str_replace('_', ' ', $date) . "</i></p>";
    echo "<p>$content</p>";
    echo "<hr>";
}
?>
