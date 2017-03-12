<?php 
$rtype=$_POST['rtype'];
$uuid=$_POST['uuid'];
// echo $rtype;
// var_dump($rtype);
switch ($rtype) {
	case '20':
		# code...
		echo checkSendMessageQueue($uuid);
		break;
	case '21':
		# code...
		echo markSendMessageQueue($uuid);
		break;
	case '22':
		echo checkRecvMessageQueue($uuid);
		break;
	case '23':
		# code...
		echo markRecvMessageQueue($uuid);
		break;
	case '24':
		# code...
		$ip=$_POST['ip'];
		$port=$_POST['port'];
		$pid=$_POST['pid'];
		$tid=$_POST['tid'];
		$ltime=$_POST['ttime'];
		$type=$_POST['type'];
		$length=$_POST['length'];
		echo recordMessageToDatabase($ip,$port,$pid,$tid,$uuid,$ltime,$type,$length);
		break;
	
	default:
		# code...
		break;
}

//add uuid to the waiting queue and check wether client can send this message.
//only when no messages are at 'sent' status and this message is at the first place of the waiting queue, this message can be sent by the client
//three kinds of message status: wait, sent, received. wait and sent status are in-queue status
function checkSendMessageQueue($uuid){
    //连接本地的 Redis 服务
	$redis = new Redis();
	$con_result=$redis->connect('127.0.0.1', 6379);
	if ($con_result){
		 $query_result=$redis->sIsMember('idset',$uuid);
		 if (!$query_result){
		 	$redis->sAdd('idset',$uuid);
		 	$redis->lPush('idlist',$uuid);
		 	$redis->hSet('idhash',$uuid,'wait');
		 }
		 $first_message=$redis->lGet('idlist',-1);
		 if ($first_message==$uuid && $redis->hGet('idhash',$uuid)=='wait'){
		 	return 0;// allow this message to be sent
		 }else{
		 	return 2;//not allowed
		 }
	}else{
		 return 1;
	}

}

//mark this message as sent in waiting queue
function markSendMessageQueue($uuid){
	$redis = new Redis();
	$con_result=$redis->connect('127.0.0.1', 6379);
	if ($con_result){
		 $query_result=$redis->sIsMember('idset',$uuid);
		 if ($query_result){
		 	$first_message=$redis->lGet('idlist',-1);
		 	if ($first_message==$uuid && $redis->hGet('idhash',$uuid)=='wait'){
			 	$redis->hSet('idhash',$uuid,'sent');
		 		return 0;
		 	}else{
		 		return 3;//this message should not be marked as sent
		 	}
		 	
		 }else{
		 	return 2;//illegal message, not exist in the waiting queue
		 }
	}else{
		 return 1;
	}
}

//check wether this message is at the the first place of waiting queue and at sent status
function checkRecvMessageQueue($uuid){
	$redis = new Redis();
	$con_result=$redis->connect('127.0.0.1', 6379);
	if ($con_result){
		 $query_result=$redis->sIsMember('idset',$uuid);
		 if ($query_result){
		 	$first_message=$redis->lGet('idlist',-1);
		 	if ($first_message==$uuid && $redis->hGet('idhash',$uuid)=='sent'){
		 		return 0;
		 	}else{
		 		return 3;//this message is not the message that should be received now
		 	}
		 	
		 }else{
		 	return 2;//illegal message, not exist in the waiting queue
		 }
	}else{
		 return 1;
	}
}

//mark this message as received and delete it from the waiting queue
function markRecvMessageQueue($uuid){
	$redis = new Redis();
	$con_result=$redis->connect('127.0.0.1', 6379);
	if ($con_result){
		 $query_result=$redis->sIsMember('idset',$uuid);
		 if ($query_result){
		 	$first_message=$redis->lGet('idlist',-1);
		 	if ($first_message==$uuid && $redis->hGet('idhash',$uuid)=='sent'){
		 		$redis->rPop('idlist');
		 		$redis->sMove('idset','deletedidset',$uuid); //move the sent message out 
			 	$redis->hSet('idhash',$uuid,'received');
		 		return 0;
		 	}else{
		 		return 3;//this message should not be marked as received
		 	}
		 	
		 }else{
		 	return 2;//illegal message, not exist in the waiting queue
		 }
	}else{
		return 1;
	}
}

//record this message to the local database, add event parents
//char* ip,int port,pid_t pid,pthread_t tid,char* uuid,long long time,char type,int length
function recordMessageToDatabase($ip,$port,$pid,$tid,$uuid,$ltime,$type,$length){
	$con = mysqli_connect("localhost","root","test1234",'trace');
	if (mysqli_connect_errno($con))
	  {
	  die('Could not connect: '.mysqli_connect_error());
	}
	mysqli_autocommit($con,FALSE);  
	// mysql_query("BEGIN");
	$query="insert into trace_info(ip,port,data_id,pid,tid,type,ltime,length) values('$ip',$port,'$uuid',$pid,$tid,$type,$ltime,$length)";
	// echo $query;
	$result=mysqli_query($con,$query);
	// mysql_query("COMMIT");
	mysqli_commit($con);
	if($result){
		mysqli_close($con);
		return 0;
	}else{
		mysqli_close($con);
		return 1;
	}

}

?>