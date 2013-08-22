<?php

function import($file,$dlc)
{
	$res = 0;
	global $db;
	$f=file_get_contents($file);
	if($f===FALSE)
		die("Error loading $file");
	$c=explode("\n",$f);
	$tb = 'arefs';
	createTable($tb);

	foreach($c as $t)
	{
		list($empty,$eid,$ref,$base,$count,$health,$cell,$x,$y,$z,$ax,$ay,$az,$flags,$lock,$key,$link)=explode("|",$t);
	
		$ref=hexdec($ref);
		$base=hexdec($base);
		$cell=hexdec($cell);
		$flags=hexdec($flags);


if (!$ref)
	continue;

                $prep = $db->prepare("insert into $tb (editor,refID,baseID,cell,x,y,z,ax,ay,az,flags,dlc) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
                if ($prep === FALSE) {
			echo "Fail: " . $db->errorInfo()[2];
			continue;
		}
		$r = $prep->execute(array($eid,$ref,$base,$cell,$x,$y,$z,$ax,$ay,$az,$flags,$dlc));
if ($r)
$res++;
	}
return $res;
}

function createTable($tb)
{
	global $db;
	$db->exec("CREATE TABLE IF NOT EXISTS $tb (editor varchar(128),refID integer, baseID integer, cell integer,x float,y float,z float,ax float,ay float, az float, flags integer,dlc integer)");
}

$db = new PDO('sqlite:fallout3.sqlite');
if(!$db)
{
    die("Errore Sqlite");
}

$dat = 0;
$dat += import("data3/f3_main_achr.txt",0);
$dat += import("data3/f3_tp_achr.txt",1);
$dat += import("data3/f3_oa_achr.txt",2);
$dat += import("data3/f3_bsn_achr.txt",3);
$dat += import("data3/f3_pl_achr.txt",4);
$dat += import("data3/f3_mz_achr.txt",5);

echo "Imported " . $dat;

//Remove duplicates
$arr=array("");
