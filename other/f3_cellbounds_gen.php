<?php



function import($file,$dlc)
{
	$maxx=array();
	$maxy=array();
	$maxz=array();
	$minx=array();
	$miny=array();
	$minz=array();
	

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

		if (!$ref)
			continue;

		if(!isset($maxx[$cell]))
		{
			$maxx[$cell]=-999999;
		}
		if(!isset($maxy[$cell]))
		{
			$maxy[$cell]=-999999;
		}
		if(!isset($maxz[$cell]))
		{
			$maxz[$cell]=-999999;
		}
		
		if(!isset($minx[$cell]))
		{
			$minx[$cell]=999999;
		}
		if(!isset($miny[$cell]))
		{
			$miny[$cell]=999999;
		}
		if(!isset($minz[$cell]))
		{
			$minz[$cell]=999999;
		}
		
		if($maxx[$cell]<$x)
		{
			$maxx[$cell]=$x;
		}
		if($maxy[$cell]<$y)
		{
			$maxy[$cell]=$y;
		}
		if($maxz[$cell]<$z)
		{
			$maxz[$cell]=$z;
		}
		
		if($minx[$cell]>$x)
		{
			$minx[$cell]=$x;
		}
		if($miny[$cell]>$y)
		{
			$miny[$cell]=$y;
		}
		if($minz[$cell]>$z)
		{
			$minz[$cell]=$z;
		}
		
		
		
		
		$res++;
	}
	
	$f2=str_replace("refr","cellbounds",$file);
	unlink($f2);
	
	$out="";
	foreach($maxx as $key=>$val)
	{
		$out.="$key|".$maxx[$key]."|".$maxy[$key]."|".$maxz[$key]."|".$minx[$key]."|".$miny[$key]."|".$minz[$key]."\r\n";
	}
	file_put_contents($f2,$out);
	
	return $res;
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
