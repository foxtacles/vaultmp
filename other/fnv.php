<?php

function import($file,$dlc)
{
	$res = 0;
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
                $prep = $db->prepare("insert into $tb (baseID,name,description,dlc) values (?, ?, ?, ?)");
                if ($prep === FALSE) {
			echo "Fail: " . $id;
			continue;
		}
		$r = $prep->execute(array($id, $name, $description, $dlc));
if ($r)
$res++;
	}
return $res;
}

function createTable($tb)
{
	global $db;
	$db->exec("CREATE TABLE $tb (baseID integer(11),name varchar(128),description varchar(128),dlc integer(11))");
}

$db = new PDO('sqlite:newvegas.sqlite');
if(!$db)
{
    die("Errore Sqlite");
}

$dat = 0;
$dat += import("data/base.TXT",0);
$dat += import("data/dm.TXT",1);
$dat += import("data/hh.TXT",2);
$dat += import("data/owb.TXT",3);
$dat += import("data/lone.TXT",4);
$dat += import("data/gun.TXT",5);

$dat += import("data/classic.TXT",6);
$dat += import("data/caravan.TXT",7);
$dat += import("data/tribal.TXT",8);
$dat += import("data/mercenary.TXT",9);

echo "Imported " . $dat;

//Remove duplicates
$arr=array("");
