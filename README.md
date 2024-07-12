# ReseauSocial
Il s'agit d'un projet mené durant six semaines dans le cadre de mon stage de première année de BTS CIEL IR au sein du Bureau Projet Ingénierie de la Direction Locale de Brest de la Direction Interarmée des Réseaux d'Infrastructure des Systèmes d'Informations.

## But 
Le but de ce projet est de créer un embryon de réseau social de type Twitter sous la forme d'une WebApp pour mobiles. Cette WebApp doit pouvoir être être testée sur un environnement de test (PC sous Debian) et mise en production sur un Raspberry Pi 2 (avec toutes les contraintes de l'environnement embarqué).

Pour pouvoir rejoindre le réseau social, les utilisateurs doivent créer un compte qui sera validé par l'administrateur. On retrouve des salons gérés par leur créateur qui peut ajouter ou supprimer des membres. Les utilisateurs peuvent publier des posts pouvant contenir un message, une image, un lien YouTube, un lien DailyMotion et une autre intégration (blog par exemple).

## Technologies utilisées.

<ul>
  <li>NGINX</li>
  <li>Scripts CGI en C</li>
  <li>JQuery Mobile (fin de support en 2021)</li>
  <li>MariaDB</li>
  <li>Google Authenticator (HMAC-SHA1)</li>
</ul>

## Risques de sécurité
Le développement d'une webapp avec des scripts CGI provoque l'apparition de différents risques de sécurité qui ont été traités :
<ul>
  <li>Dépassement de variables -> Allocation dynamique dépendant de la taille des données transmises</li>
  <li>Injection SQL -> Transformation des caractères ' et " en \' pour les échapper</li>
  <li>Cross-Site Scripting (XSS) -> Transformation des caractères &lt; en &amp;lt; </li>
  <li>Exécution indépendante des scripts CGI -> Authentification à chaque début de script</li>
  <li>Transfert de données -> Utilisation de l'HTTPS</li>
</ul>
