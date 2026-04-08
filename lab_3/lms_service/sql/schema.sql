DROP TABLE IF EXISTS lesson_progress CASCADE;
DROP TABLE IF EXISTS course_enrollments CASCADE;
DROP TABLE IF EXISTS lessons CASCADE;
DROP TABLE IF EXISTS courses CASCADE;
DROP TABLE IF EXISTS users CASCADE;

CREATE TABLE users (
    id BIGSERIAL PRIMARY KEY,
    login VARCHAR(100) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    first_name VARCHAR(100) NOT NULL,
    last_name VARCHAR(100) NOT NULL,
    role VARCHAR(50) NOT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT NOW(),
    CHECK (char_length(login) >= 3)
);

CREATE TABLE courses (
    id BIGSERIAL PRIMARY KEY,
    title VARCHAR(200) NOT NULL,
    description TEXT,
    teacher_id BIGINT NOT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT NOW(),
    CONSTRAINT fk_courses_teacher
        FOREIGN KEY (teacher_id) REFERENCES users(id)
        ON DELETE RESTRICT,
    CONSTRAINT chk_courses_title_length
        CHECK (char_length(title) >= 2)
);

CREATE TABLE lessons (
    id BIGSERIAL PRIMARY KEY,
    course_id BIGINT NOT NULL,
    title VARCHAR(200) NOT NULL,
    content TEXT,
    position INTEGER NOT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT NOW(),
    CONSTRAINT fk_lessons_course
        FOREIGN KEY (course_id) REFERENCES courses(id)
        ON DELETE CASCADE,
    CONSTRAINT chk_lessons_position
        CHECK (position > 0),
    CONSTRAINT uq_lessons_course_position
        UNIQUE (course_id, position)
);

CREATE TABLE course_enrollments (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL,
    course_id BIGINT NOT NULL,
    enrolled_at TIMESTAMP NOT NULL DEFAULT NOW(),
    CONSTRAINT fk_course_enrollments_user
        FOREIGN KEY (user_id) REFERENCES users(id)
        ON DELETE CASCADE,
    CONSTRAINT fk_course_enrollments_course
        FOREIGN KEY (course_id) REFERENCES courses(id)
        ON DELETE CASCADE,
    CONSTRAINT uq_course_enrollments_user_course
        UNIQUE (user_id, course_id)
);

CREATE TABLE lesson_progress (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL,
    lesson_id BIGINT NOT NULL,
    is_completed BOOLEAN NOT NULL DEFAULT FALSE,
    completed_at TIMESTAMP,
    CONSTRAINT fk_lesson_progress_user
        FOREIGN KEY (user_id) REFERENCES users(id)
        ON DELETE CASCADE,
    CONSTRAINT fk_lesson_progress_lesson
        FOREIGN KEY (lesson_id) REFERENCES lessons(id)
        ON DELETE CASCADE,
    CONSTRAINT uq_lesson_progress_user_lesson
        UNIQUE (user_id, lesson_id)
);

CREATE INDEX idx_courses_teacher_id
    ON courses(teacher_id);

CREATE INDEX idx_course_enrollments_course_id
    ON course_enrollments(course_id);

CREATE INDEX idx_lesson_progress_lesson_id
    ON lesson_progress(lesson_id);

CREATE INDEX idx_users_last_name_first_name
    ON users(last_name, first_name);