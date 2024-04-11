<?php
/**
 * The base configuration for WordPress
 *
 * The wp-config.php creation script uses this file during the installation.
 * You don't have to use the web site, you can copy this file to "wp-config.php"
 * and fill in the values.
 *
 * This file contains the following configurations:
 *
 * * Database settings
 * * Secret keys
 * * Database table prefix
 * * Localized language
 * * ABSPATH
 *
 * @link https://wordpress.org/support/article/editing-wp-config-php/
 *
 * @package WordPress
 */

// ** Database settings - You can get this info from your web host ** //
/** The name of the database for WordPress */
define( 'DB_NAME', 'wordpress' );

/** Database username */
define( 'DB_USER', 'hyungjuk' );

/** Database password */
define( 'DB_PASSWORD', 'secret' );

/** Database hostname */
define( 'DB_HOST', 'mariadb' );

/** Database charset to use in creating database tables. */
define( 'DB_CHARSET', 'utf8' );

/** The database collate type. Don't change this if in doubt. */
define( 'DB_COLLATE', '' );

/**#@+
 * Authentication unique keys and salts.
 *
 * Change these to different unique phrases! You can generate these using
 * the {@link https://api.wordpress.org/secret-key/1.1/salt/ WordPress.org secret-key service}.
 *
 * You can change these at any point in time to invalidate all existing cookies.
 * This will force all users to have to log in again.
 *
 * @since 2.6.0
 */
define( 'AUTH_KEY',          'gVk-*gC15z3)9@aO.pJBvM>CQ3@?Ys.WA*?7mP9wyzGgjirQTZBbQ1{wah] 4vE{' );
define( 'SECURE_AUTH_KEY',   '<d=(u|%;5^%4m;<76T,mn;!VoL!2KG*{vw_rk0%;68Zo.j!pK5r]n9YJ|v^?E{>#' );
define( 'LOGGED_IN_KEY',     '6an47JXM9-7[?0!9[-GJfKU&Nzrej4nT]+Ve=3HQWUP=k(oI1Ex3}`Po_FCbGCzu' );
define( 'NONCE_KEY',         '}p;]*H+LLR2&6If|Zdks*pS 9[mk4Tp9OIB@v0-25W|RmD?kq]EN:A6PoFPM4TlQ' );
define( 'AUTH_SALT',         ']5oV80SSqUd)@)P6e6:HZNBCt>GT K*:lM|A3}XUvTKR2Ss$}!|V%Gky6)a.sFxk' );
define( 'SECURE_AUTH_SALT',  'h]vAHX0T<obmc},aA~V>|=mW 5?f(tV)7@`At@2W]:YrlN<y]oLXv2*O5ZQsBY<1' );
define( 'LOGGED_IN_SALT',    'Yg`p>J}P[dpk=/jBdLsO7pYj8Ftdd0c,/bnJoFOYc^fUpo&WxVt7 +ETUdQ~{!p1' );
define( 'NONCE_SALT',        '=,1$2spigmJVid6T=L;W2?F$r~[__`u^-52hC;z`O]q^+]sh /.d1A+L`0)nz s8' );
define( 'WP_CACHE_KEY_SALT', '9td@.Y>vc(L*z| /w[`w!w`L> a(8#WV~ht],a>sT(8W .Dw|FCmAhwm4=m80hms' );


/**#@-*/

/**
 * WordPress database table prefix.
 *
 * You can have multiple installations in one database if you give each
 * a unique prefix. Only numbers, letters, and underscores please!
 */
$table_prefix = 'wp_';


/* Add any custom values between this line and the "stop editing" line. */



/**
 * For developers: WordPress debugging mode.
 *
 * Change this to true to enable the display of notices during development.
 * It is strongly recommended that plugin and theme developers use WP_DEBUG
 * in their development environments.
 *
 * For information on other constants that can be used for debugging,
 * visit the documentation.
 *
 * @link https://wordpress.org/support/article/debugging-in-wordpress/
 */
if ( ! defined( 'WP_DEBUG' ) ) {
	define( 'WP_DEBUG', true );
}

define( 'WP_DEBUG_LOG', true );
define( 'WP_REDIS_HOST', 'redis' );
define( 'WP_CACHE', true );
/* That's all, stop editing! Happy publishing. */

/** Absolute path to the WordPress directory. */
if ( ! defined( 'ABSPATH' ) ) {
	define( 'ABSPATH', __DIR__ . '/' );
}

/** Sets up WordPress vars and included files. */
require_once ABSPATH . 'wp-settings.php';
