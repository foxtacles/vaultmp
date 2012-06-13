<?php

function import($file,$dlc)
{
	global $db;
	$f=file_get_contents($file);
	if($f===FALSE)
		die("Error loading $file");
	$c=explode("\n",$f);
	foreach($c as $t)
	{
		list($id,$name,$tb,$pad,$description)=explode("\t",$t);

		$id=hexdec(trim(str_replace("FormID: ","",$id)));
		createTable($tb);
		$description=trim($description);
		$db->exec("insert into $tb (baseID,name,description,dlc) values ('$id','$name','$description','$dlc')");
	}
}

function createTable($tb)
{
	global $db;
	$db->exec("CREATE TABLE $tb (baseID integer(11) PRIMARY KEY,name varchar(128),description varchar(128),dlc integer(11))");
}

$db = new PDO('sqlite:/home/recycler/www/newvegas.sqlite');
if(!$db)
{
    die("Errore Sqlite");
}

import("data/base.TXT",0);
import("data/dm.TXT",1);
import("data/hh.TXT",2);
import("data/owb.TXT",3);
import("data/lone.TXT",4);
import("data/gun.TXT",5);

import("data/classic.TXT",6);
import("data/caravan.TXT",7);
import("data/tribal.TXT",8);
import("data/mercenary.TXT",9);

//Remove duplicates
$arr=array("");
