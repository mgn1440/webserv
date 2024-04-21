<?php
// Retrieve the query string from the QUERY_STRING environment variable
$queryString = getenv('QUERY_STRING');

// Parse the query string into an array
parse_str($queryString, $queryParams);

// Retrieve the desired value (e.g., name) from the queryParams array
$name = isset($queryParams['name']) ? $queryParams['name'] : '방문자';

// Use htmlspecialchars for safe HTML output
$name = htmlspecialchars($name, ENT_QUOTES, 'UTF-8');
?>

<!DOCTYPE html>
<html lang="ko">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>인사하기</title>
</head>
<body>
    <h1>안녕하세요, <?php echo $name; ?>님!</h1>
</body>
</html>
