/*
CREATE DATABASE `trace` CHARACTER SET utf8 COLLATE utf8_general_ci;
use trace;
*/
drop table if exists `trace_info`;
create table `trace_info`(
	`id` int not null primary key auto_increment,
	`on_ip` text not null,
	`on_port` int not null,
	`in_ip` text not null,
	`in_port` int not null,
	`data_id` text not null,
	`data_unit_id` text not null,
	`message` text default null,
	`message2` text default null,
	`connect_id` int,
	`connect_uuid` varchar(255),
	`pid` int not null,
	`ktid` int not null,
	`tid` bigint not null,
	`ftype` tinyint not null,
	`dtype` tinyint not null,
	`time` timestamp default CURRENT_TIMESTAMP,
	`ltime` bigint(20) not null,
	`supposed_length` bigint(20) not null,
	`length` bigint(20) not null,
	`rlength` bigint(20) not null,
	`source_ip` VARCHAR(255)
);

drop table if exists `request_type`;
create table `request_type`(
	`id` int not null primary key auto_increment,
	`number` int not null,
	`description` varchar(255)
);
 
insert into `request_type`(number,description) values (20,'check_send');
insert into `request_type`(number,description) values (21,'mark_send');
insert into `request_type`(number,description) values (22,'check_recv');
insert into `request_type`(number,description) values (23,'mark_recv');
insert into `request_type`(number,description) values (24,'push_database');
insert into `request_type`(number,description) values (25,'open_socket');
insert into `request_type`(number,description) values (26,'close_socket');
insert into `request_type`(number,description) values (27,'push_thread');
insert into `request_type`(number,description) values (28,'push_thread_dep');


drop table if exists `func_type`;
create table `func_type`(
	`id` int not null primary key auto_increment,
	`number` int not null,
	`description` varchar(255)
);
insert into `func_type`(number,description) values (1,'send'); 
insert into `func_type`(number,description) values (2,'recv');
insert into `func_type`(number,description) values (3,'write');
insert into `func_type`(number,description) values (4,'read');
insert into `func_type`(number,description) values (5,'sendmsg');
insert into `func_type`(number,description) values (6,'recvmsg');
insert into `func_type`(number,description) values (7,'sendto');
insert into `func_type`(number,description) values (8,'recvfrom');
insert into `func_type`(number,description) values (9,'writev');
insert into `func_type`(number,description) values (10,'readv');
insert into `func_type`(number,description) values (11,'sendmmsg');
insert into `func_type`(number,description) values (12,'recvmmsg');
insert into `func_type`(number,description) values (13,'connect');
insert into `func_type`(number,description) values (14,'socket');
insert into `func_type`(number,description) values (15,'close');
insert into `func_type`(number,description) values (17,'sendfile64');
insert into `func_type`(number,description) values (19,'sendfile');
insert into `func_type`(number,description) values (20,'accept');
insert into `func_type`(number,description) values (21,'fork');
insert into `func_type`(number,description) values (22,'pthread_cerate');


drop table if exists `finish_type`;
create table `finish_type`(
	`id` int not null primary key auto_increment,
	`number` int not null,
	`description` varchar(255)
);
insert into `finish_type`(number,description) values (1,'send done normally');
insert into `finish_type`(number,description) values (2,'send done error');
insert into `finish_type`(number,description) values (3,'send done with uuid');
insert into `finish_type`(number,description) values (4,'send done with filter');
insert into `finish_type`(number,description) values (5,'send done with udp');
insert into `finish_type`(number,description) values (6,'send done with broken socket');
insert into `finish_type`(number,description) values (7,'send done with failure');
insert into `finish_type`(number,description) values (8,'send done with other situation');

insert into `finish_type`(number,description) values (9,'recv done normally');
insert into `finish_type`(number,description) values (10,'recv done with filter');
insert into `finish_type`(number,description) values (11,'recv done with left bytes');
insert into `finish_type`(number,description) values (12,'recv done with failure');
insert into `finish_type`(number,description) values (13,'recv done with single and broken uuid');
insert into `finish_type`(number,description) values (14,'recv done with uuid');
insert into `finish_type`(number,description) values (15,'recv done with error');
insert into `finish_type`(number,description) values (16,'recv done with head error');
insert into `finish_type`(number,description) values (17,'recv done with head failure');
insert into `finish_type`(number,description) values (18,'done with IPv6');
insert into `finish_type`(number,description) values (19,'done with UNIX socket');
insert into `finish_type`(number,description) values (20,'done with other sockets');

drop table if exists `thread_dep`;
create table `thread_dep`(
	`id` int not null primary key auto_increment,
	`process_id` int not null,
	`k_thread_id` int not null,
	`d_thread_id` bigint not null,
	`j_thread_id` bigint not null,
	`time` timestamp default CURRENT_TIMESTAMP,
	`ltime` bigint(20) not null,
	`data_unit_id` text,
	`source_ip` VARCHAR(255)
);

drop table if exists `threads`;
create table `threads`(
	`id` int not null primary key auto_increment,
	`process_id` int not null,
	`k_thread_id` int not null,
	`thread_id` bigint not null,
	`p_thread_id` bigint,
	`pk_thread_id` int not null,
	`p_process_id` int default 0,
	`time` timestamp default CURRENT_TIMESTAMP,
	`ltime` bigint(20) not null,
	`deleted` int default 0,
	`t_family` int,
	`t_parent` int,
	`ind_t_family` int,
	`ind_t_parent` int,
	`ind_parent` int,
	`data_unit_id` text,
	`source_ip` VARCHAR(255)
);

drop table if exists userlogs;
create table userlogs(
	`id` bigint(20) not null primary key auto_increment,
	`uuid` text not null,
	`ktid` int,
	`level` text,
	`time` VARCHAR(255) default null,
	`ltime` bigint(20) default 0,
	`loginfo` text default null,
	`label` int,
	`filename` text,
	`source_ip` VARCHAR(255)
);


drop table if exists other_event;
create table other_event(
	`id` bigint(20) not null PRIMARY key auto_increment,
	`ftype` int,
	`result` int,
	`ltime` bigint(20) default 0,
	`pid` int,
	`ktid` int,
	`tid` bigint,
	`data_unit_id` text,
	`source_ip` varchar(255)
	);

drop table if exists all_events;
create table all_events(
	`id` bigint(20) not null PRIMARY key auto_increment,
	`source_type` TINYINT,
	`event_id` bigint(20) not NULL,
	`ftype` TINYINT,
	`data_id` text,
	`data_unit_id` text,
	`pid` int,
	`ktid` int,
	`tid` bigint,
	`ltime` bigint(20) default 0,
	`result` int,
	`source_ip` VARCHAR(255)
);

drop table if exists all_trace_events;
create table all_trace_events(
	`id` bigint(20) not null PRIMARY key auto_increment,
	`source_type` TINYINT,
	`event_id` bigint(20) not NULL,
	`ftype` TINYINT,
	`data_id` text,
	`data_unit_id` text,
	`pid` int,
	`ktid` int,
	`tid` bigint,
	`ltime` bigint(20) default 0,
	`result` int,
	`source_ip` VARCHAR(255)
);

drop table if exists combination1;
create table combination1(
	`id` bigint(20) not null PRIMARY key auto_increment,
	`source_type` TINYINT,
	`event_id` bigint(20) not NULL,
	`ftype` TINYINT,
	`data_id` text,
	`data_unit_id` text,
	`pid` int,
	`ktid` int,
	`tid` bigint,
	`ltime` bigint(20) default 0,
	`result` int,
	`source_ip` VARCHAR(255)
);

drop table if exists combination2;
create table combination2(
	`id` bigint(20) not null PRIMARY key auto_increment,
	`source_type` TINYINT,
	`event_id` bigint(20) not NULL,
	`ftype` TINYINT,
	`data_id` text,
	`data_unit_id` text,
	`pid` int,
	`ktid` int,
	`tid` bigint,
	`ltime` bigint(20) default 0,
	`result` int,
	`source_ip` VARCHAR(255)
);

drop table if exists abstract1;
create table abstract1(
	`id` bigint(20) not null PRIMARY key auto_increment,
	`data_unit_id` text,
	`ktid` int,
	`source_ip` VARCHAR(255),
	`internal_length` int,
	`child_number` int,
	`child_total` int,
	`parent_unit` bigint(20),
	`f_sequence` text,
	`source_sequence` text
);

drop table if exists abstract2;
create table abstract2(
	`id` bigint(20) not null PRIMARY key auto_increment,
	`data_unit_id` text,
	`ktid` int,
	`source_ip` VARCHAR(255),
	`internal_length` int,
	`child_number` int,
	`child_total` int,
	`parent_unit` bigint(20),
	`f_sequence` text,
	`source_sequence` text
);




