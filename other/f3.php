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

$db = new PDO('sqlite:fallout3.sqlite');
if(!$db)
{
    die("Errore Sqlite");
}

$dat = 0;
$dat += import("data3/base.TXT",0);
$dat += import("data3/pitt.TXT",1);
$dat += import("data3/anchor.TXT",2);
$dat += import("data3/bs.TXT",3);
$dat += import("data3/pl.TXT",4);
$dat += import("data3/ze.TXT",5);

echo "Imported " . $dat;

//Remove duplicates
$arr=array("");
