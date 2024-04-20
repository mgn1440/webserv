# request ends with slash
    - directory => find index

# root directory and index is set
    - root + location path + index file

# if "//" is in path, must be an error => too many redir

www.google.com/html/index.html

=> /var/www/html/index.html

server {
    root /var;      

    location /html {
        root /var/www/;
        index index.php;
        // /html => /var/www/html/index.php;
    }
}