# enrollment
delimiter $$
DROP PROCEDURE IF EXISTS enroll_course$$
CREATE PROCEDURE enroll_course (IN coursecode char(8), IN StudentId int(11), IN courseyear int(11), IN coursesemester char(2)) 
BEGIN
	DECLARE error int DEFAULT 0;
	DECLARE status INT DEFAULT 0;
	DECLARE exit handler FOR sqlexception SET error = 1;
    DECLARE exit handler FOR sqlwarning SET error = 1;

    START TRANSACTION;
    SET error = 0;

	# max_enrollment
	IF coursecode IN (SELECT c.uoscode FROM uosoffering AS c WHERE c.enrollment >= c.maxenrollment)
	THEN SET status = 1;
		 SELECT status;

	# prerequiste
	-- ELSEIF coursecode NOT IN (SELECT r.uoscode FROM requires AS r WHERE r.prerequoscode IN (SELECT t.uoscode FROM transcript AS t WHERE t.studid = StudentId AND (t.grade = 'P' or t.grade = 'CR')))
	ELSEIF EXISTS(SELECT * FROM requires WHERE UoSCode = coursecode 
											AND (PrereqUoSCode NOT IN (SELECT UoSCode FROM transcript WHERE StudId = studentId)
                                                 OR PrereqUoSCode IN (SELECT UoSCode FROM transcript WHERE StudId = studentId AND Grade is NULL)
                                                 OR PrereqUoSCode IN (SELECT UoSCode FROM transcript WHERE StudId = studentId AND Grade = "D")))
	THEN SET status = 2;
		 SELECT status;

	# course_exists
	ELSEIF coursecode IN (SELECT t.uoscode FROM transcript AS t WHERE t.studid = StudentId)
	THEN SET status = 3;
		 SELECT status;

	# success
	ELSE
		INSERT INTO transcript VALUES (StudentID, courseCode, coursesemester, courseyear, NULL);
		UPDATE uosoffering SET enrollment = enrollment + 1 WHERE uoscode = coursecode AND year = courseyear AND semester = coursesemester;
		SET status = 4;
		SELECT status;
	END IF;
	

	IF error = 1
	THEN ROLLBACK;
	ELSE
		COMMIT;
	END IF;
	
End $$
delimiter ;


# withdraw
delimiter $$
DROP PROCEDURE IF EXISTS withdraw_course$$
CREATE PROCEDURE withdraw_course (IN coursecode char(8), IN studentId int(11))
BEGIN
	DECLARE status int default 0;
	DECLARE error int default 0;
    DECLARE exit handler FOR sqlexception SET error = 1;
    DECLARE exit handler FOR sqlwarning SET error = 1;
	
	START TRANSACTION;
	SET error = 0;

	# course_not_enrolled
	IF coursecode not IN (SELECT t.uoscode FROM transcript AS t WHERE t.studid = studentId)
	THEN SET status = 1;
		 SELECT status;

	# course_with_grade
	ELSEIF coursecode IN (SELECT t.uoscode FROM transcript AS t WHERE t.studid = studentId AND t.grade is NOT NULL)
	THEN SET status = 2;
		 SELECT status;

	# success
	ELSE
		SET status = 3;
		DELETE FROM transcript WHERE studid = studentId AND uoscode = coursecode;
		UPDATE uosoffering SET enrollment = enrollment - 1 WHERE uoscode = coursecode;
		SELECT status;
	END IF;
	
	IF error = 1
	THEN ROLLBACK;
	ELSE
		COMMIT;
	END IF;

END $$
delimiter ;



DROP TABLE IF EXISTS warning_log;
DROP TRIGGER IF EXISTS half_MaxEnrollment;
CREATE TABLE warning_log(warning INT, _time datetime);
delimiter $$
CREATE TRIGGER half_MaxEnrollment AFTER UPDATE ON uosoffering FOR EACH ROW
BEGIN
	DECLARE warning INT DEFAULT 0;
	IF new.enrollment< 0.5 * old.Maxenrollment THEN
		INSERT INTO warning VALUES (1, NOW()); 
	ELSE
		DELETE FROM warning_log;
	END IF;
end $$
delimiter ;