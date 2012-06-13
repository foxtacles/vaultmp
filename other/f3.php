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

$db = new PDO('sqlite:fallout3.sqlite');
if(!$db)
{
    die("Errore Sqlite");
}

import("data3/base.TXT",0);
import("data3/pitt.TXT",1);
import("data3/anchor.TXT",2);
import("data3/bs.TXT",3);
import("data3/pl.TXT",4);
import("data3/ze.TXT",5);

//Remove duplicates
$arr=array("");
