<?php
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $postsDir = "./blog/php/txt";
	if (!is_dir($postsDir)) {
		mkdir($postsDir, 0777, true);
	}

	$input = fopen('php://stdin', 'r');
	$line = fgets($input);

	parse_str($line, $post);
	$title = $post['title'];
	$content = $post['content'];
	$date = date('Y-m-d H:i:s');
	$filename = $postsDir . '/' . $date . '_' . str_replace(' ', '_', $title) . '.txt';
	$post = "$content";
	file_put_contents($filename, $post);
	echo "Post published successfully. <a href='index.php'>Go back to posts list</a>";
} else {
    ?>
    <form method="POST">

        Title: <input type="text" name="title"><br>
        Content:<br>
        <textarea name="content"></textarea><br>
        <input type="submit" value="Publish">
    </form>
    <?php
}
?>
