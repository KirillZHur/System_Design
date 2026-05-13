-- 1. Создание нового пользователя
INSERT INTO users (login, password_hash, first_name, last_name, role)
VALUES ('new_user', 'hashed_password', 'Ivan', 'Petrov', 'student')
RETURNING id, login, first_name, last_name, role, created_at;

-- 2. Поиск пользователя по логину
SELECT id, login, first_name, last_name, role, created_at
FROM users
WHERE login = 'ivan.petrov';

-- 3. Поиск пользователя по маске имени и фамилии
SELECT id, login, first_name, last_name, role, created_at
FROM users
WHERE last_name ILIKE 'Пет%'
  AND first_name ILIKE 'Ив%'
ORDER BY id;

-- 4. Создание курса
INSERT INTO courses (title, description, teacher_id)
VALUES ('Databases', 'Курс по базам данных', 1)
RETURNING id, title, description, teacher_id, created_at;

-- 5. Получение списка курсов
SELECT id, title, description, teacher_id, created_at
FROM courses
ORDER BY id;

-- 6. Добавление урока в курс
INSERT INTO lessons (course_id, title, content, position)
VALUES (1, 'Normalization', 'Первая, вторая и третья нормальные формы', 3)
RETURNING id, course_id, title, content, position, created_at;

-- 7. Получение уроков курса
SELECT id, course_id, title, content, position, created_at
FROM lessons
WHERE course_id = 1
ORDER BY position;

-- 8. Запись пользователя на курс
INSERT INTO course_enrollments (user_id, course_id)
VALUES (1, 2)
ON CONFLICT (user_id, course_id) DO NOTHING
RETURNING id, user_id, course_id, enrolled_at;

-- 9. Получение курсов пользователя
SELECT
    c.id,
    c.title,
    c.description,
    c.teacher_id,
    ce.enrolled_at
FROM course_enrollments ce
JOIN courses c ON c.id = ce.course_id
WHERE ce.user_id = 1
ORDER BY ce.enrolled_at DESC;

-- 10. Отметка о прохождении урока
INSERT INTO lesson_progress (user_id, lesson_id, is_completed, completed_at)
VALUES (1, 1, TRUE, NOW())
ON CONFLICT (user_id, lesson_id)
DO UPDATE SET
    is_completed = EXCLUDED.is_completed,
    completed_at = EXCLUDED.completed_at
RETURNING id, user_id, lesson_id, is_completed, completed_at;