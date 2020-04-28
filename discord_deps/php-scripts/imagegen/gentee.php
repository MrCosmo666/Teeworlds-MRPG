<?php
	// по умолчанию переменные
	$player = "Refresh";
	$rank = "0";
	$discordcard = "-1";

	// ищет последнее вхождение символа
	function StrCharPosBack($haystack, $needle) 
	{
		for ($i = strlen($haystack) ; $i>=0 ; $i--)
		{
			if (substr($haystack, $i, 1) === $needle)
				return $i;
		}
		return false;
	}
	
	$str = "{I have|I love|Santa Barbara|Big|Small|Medium} {Tee|Bird|Grandmother|Fantasy|Tiger Arr..}";
	while (strpos($str, "{") !== false)
	{
		$pos1 = strpos($str, "}");
		$buf_mas = substr($str, 0, $pos1);
		$pos2 = StrCharPosBack ($buf_mas, "{");
		$buf_mas = substr($buf_mas, $pos2+1);
		$exploded_buf_mas = explode ("|", $buf_mas);
		$rnd = rand(0, count($exploded_buf_mas)-1);
		$str = substr_replace($str, $exploded_buf_mas[$rnd], $pos2, $pos1+1-$pos2);
	}

	if (isset($_GET['player']))
		$player = $_GET['player'];
	if (isset($_GET['dicid']))
		$discordcard = $_GET['dicid'];
	if (isset($_GET['rank']))
		$rank = $_GET['rank'];
	
	$font = "fonts/18162.ttf";
	$font_size = 40; 
	$font_size_desc = 16;

	// создаём новое изображение из файла
	$img = imagecreatefrompng("images/$discordcard.png"); 
	$whitecolor  = imagecolorallocatealpha($img, 255, 255, 255, 20);
	$randomcolor  = imagecolorallocatealpha($img, rand(140, 255), rand(140, 255), rand(140, 255), 20);
	
	// обычный текст не динамический
	imagettftext($img, $font_size_desc, 0, 20, 40, $randomcolor, $font, "DISCORD MMO TEE CARD");
	imagettftext($img, 12 + $font_size, 0, 60, 220, $randomcolor, $font, "$player : Rank #$rank");
	imagettftext($img, 6 + $font_size, 0, 60, 280, $whitecolor, $font, $str);

	// вывод изображения в браузер и уничтожение его
	header("Content-type: image/png");
	imagepng($img);
	imagedestroy($img);
?>