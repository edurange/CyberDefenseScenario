all: run_programs

compile:
	gcc -o checker_exe checker.c

run_programs: compile
	cd /tmp/scoring
	mkdir uptime
	cd uptime
	wget -O uptime_checker.zip http://bit.ly/1EqxrTg
	unzip uptime_checker.zip
	cd /tmp/scoring/uptime
	rm uptime_checker.zip
	make
	cd ..
	rm -rf uptime
	./checker_exe&
	crontab -l { cat; echo "20 * * * * /tmp/scoring/update_scoreboard.sh"; } | crontab -
