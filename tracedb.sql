drop table if exists `trace_info`;
create table `trace_info`(
	`id` int not null primary key auto_increment,
	`ip` text not null,
	`port` int not null,
	`data_id` varchar(255) not null,
	`connect_id` int,
	`connect_uuid` varchar(255),
	`pid` int not null,
	`tid` int not null,
	`type` tinyint not null,
	`time` timestamp not null,
	`ltime` bigint(20) not null,
	`length` int not null
);

