online-judge

gcc -O2 -o judge -DDEBUG judge.cpp support.cpp -lmysqlclient

gcc -O2 -o run -DDEBUG run.cpp lang.cpp support.cpp lang/lang_c.cpp lang/lang_java.cpp -lmysqlclient

#process_sql in judge.cpp:

设置judge队列

从judges表找到要判哪部分的，以及要判的部分的id
```
execsql("select status_id,judge_for from judges limit %d", RUN_MAX << 1);
```
in run.cpp:

修改状态为judging
```
execsql("UPDATE `%s_status` SET `status`=%d WHERE id=%u", prefix, 1, status_id);
```
从{judge_for}_status获取题目信息及用户信息
```
execsql("SELECT problem_id,lang,user_id FROM %s_status WHERE id=%u", prefix, status_id);
```
获取真实题目id
```
execsql("SELECT problem_id FROM %s_problems WHERE id=%u", prefix, problem_id);
```
获取题目的具体信息
```
execsql("SELECT time_limit,memory_limit,spj,judge_cnt FROM problems WHERE id=%u", problem_id);
```
获取源代码
```
execsql("SELECT code FROM %s_codes where status_id=%u", prefix, status_id);
```
设置ce信息
```
execsql("INSERT INTO `%s_ces`(`status_id`, `content`) VALUES (%u, '%s')", prefix, status_id, ce_buf);
```
修改状态
```
if(execsql("UPDATE `%s_status` SET `status`=%d WHERE id=%u", prefix, status, status_id)) {
 
   execsql("DELETE FROM `judges` WHERE status_id=%u AND judge_for=%d", status_id, judge_for);

}
```
