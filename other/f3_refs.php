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
		list($tb,$eid,$ref,$base,$count,$health,$cell,$x,$y,$z,$ax,$ay,$az,$flags,$lock,$key,$link)=explode("|",$t);
		$tb = 'refs_' . $tb;

		$ref=hexdec($ref);
		$base=hexdec($base);
		$cell=hexdec($cell);
		$flags=hexdec($flags);
		$key=hexdec($key);
		$link=hexdec($link);
		createTable($tb);

if (!$ref)
	continue;

                $prep = $db->prepare("insert into $tb (editor,refID,baseID,count,health,cell,x,y,z,ax,ay,az,flags,lock,key,link,dlc) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
                if ($prep === FALSE) {
			echo "Fail: " . $db->errorInfo()[2];
			continue;
		}
		$r = $prep->execute(array($eid,$ref,$base,$count,$health,$cell,$x,$y,$z,$ax,$ay,$az,$flags,$lock,$key,$link, $dlc));
if ($r)
$res++;
	}
return $res;
}

function createTable($tb)
{
	global $db;
	$db->exec("CREATE TABLE IF NOT EXISTS $tb (editor varchar(128),refID integer, baseID integer,count integer, health float, cell integer,x float,y float,z float,ax float,ay float, az float, flags integer, lock integer, key integer, link integer,dlc integer)");
}

$db = new PDO('sqlite:fallout3.sqlite');
if(!$db)
{
    die("Errore Sqlite");
}

$dat = 0;
$dat += import("data3/f3_main_refr.txt",0);
$dat += import("data3/f3_tp_refr.txt",1);
$dat += import("data3/f3_oa_refr.txt",2);
$dat += import("data3/f3_bsn_refr.txt",3);
$dat += import("data3/f3_pl_refr.txt",4);
$dat += import("data3/f3_mz_refr.txt",5);

echo "Imported " . $dat;

//Remove duplicates
$arr=array("");
