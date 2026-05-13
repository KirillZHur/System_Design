INSERT INTO users (login, password_hash, first_name, last_name, role, created_at) VALUES
('ivan.petrov', 'efb583d376b19d92d81e75bea335768d2b5cc9d60460c182cb6e66e8031b1aea', 'Иван', 'Петров', 'student', '2026-04-01 10:00:00'),
('anna.smirnova', 'fc2c0c139d5b71c45a339f91a81961904ec564d62ca3727e0679bef4193c7c7a', 'Анна', 'Смирнова', 'teacher', '2026-04-01 10:05:00'),
('dmitry.ivanov', '4535cb7d17e76d393a2f19cbaf5efd362114649fc3984689530b5404a3b7b63f', 'Дмитрий', 'Иванов', 'student', '2026-04-01 10:10:00'),
('elena.kozlova', '9a6c2605d514a480d532a59879cd77d293816e234c1f22ce1a0d138bcf1e8673', 'Елена', 'Козлова', 'teacher', '2026-04-01 10:15:00'),
('sergey.popov', 'a12338f804cfae81d5e4aab92741c7f282fe4cdaa4760539d1bd032b47d3db7a', 'Сергей', 'Попов', 'student', '2026-04-01 10:20:00'),
('olga.sokolova', 'a9f6bff7e48bf501fcaa14469cd989cc432441caf51e8a56ef9f1293982d3948', 'Ольга', 'Соколова', 'teacher', '2026-04-01 10:25:00'),
('nikita.volkov', 'e6d07cf00e3a65c1f1544c326c69d8df2025812fc8944891f89c58e49ad6e5c0', 'Никита', 'Волков', 'student', '2026-04-01 10:30:00'),
('maria.fedorova', '6abfe8cde5f624da2bbdfbf6c258c3ac1709f6fff78a6a3c89aafa47b7d5861b', 'Мария', 'Федорова', 'teacher', '2026-04-01 10:35:00'),
('alexey.morozov', 'e765e151e195fad7fde18d7322f963e8cba135a461617072cc576f86d79df840', 'Алексей', 'Морозов', 'student', '2026-04-01 10:40:00'),
('irina.lebedeva', 'bf5718a1c83be9a164e68da864fdae1114153ef0206bda78c3de0742027c40e7', 'Ирина', 'Лебедева', 'teacher', '2026-04-01 10:45:00');

INSERT INTO courses (title, description, teacher_id, created_at) VALUES
('C++ Basics', 'Базовый курс по C++', 2, '2026-04-02 09:00:00'),
('PostgreSQL Fundamentals', 'Основы PostgreSQL', 4, '2026-04-02 09:10:00'),
('Web Development', 'Введение в веб-разработку', 6, '2026-04-02 09:20:00'),
('Algorithms', 'Алгоритмы и структуры данных', 8, '2026-04-02 09:30:00'),
('Operating Systems', 'Основы операционных систем', 10, '2026-04-02 09:40:00'),
('Computer Networks', 'Компьютерные сети', 2, '2026-04-02 09:50:00'),
('Docker Essentials', 'Работа с Docker', 4, '2026-04-02 10:00:00'),
('Linux Administration', 'Администрирование Linux', 6, '2026-04-02 10:10:00'),
('Software Testing', 'Тестирование ПО', 8, '2026-04-02 10:20:00'),
('System Design', 'Основы проектирования систем', 10, '2026-04-02 10:30:00');

INSERT INTO lessons (course_id, title, content, position, created_at) VALUES
(1, 'Introduction to C++', 'Основные конструкции языка C++', 1, '2026-04-03 08:00:00'),
(1, 'Functions in C++', 'Функции и параметры', 2, '2026-04-03 08:10:00'),
(2, 'Installing PostgreSQL', 'Установка PostgreSQL', 1, '2026-04-03 08:20:00'),
(2, 'Basic SQL Queries', 'Простые SQL-запросы', 2, '2026-04-03 08:30:00'),
(3, 'HTML Basics', 'Основы HTML', 1, '2026-04-03 08:40:00'),
(4, 'Arrays and Lists', 'Массивы и списки', 1, '2026-04-03 08:50:00'),
(5, 'Processes and Threads', 'Процессы и потоки', 1, '2026-04-03 09:00:00'),
(6, 'OSI Model', 'Модель OSI', 1, '2026-04-03 09:10:00'),
(7, 'Docker Images', 'Образы Docker', 1, '2026-04-03 09:20:00'),
(10, 'Scalability Basics', 'Основы масштабируемости', 1, '2026-04-03 09:30:00');

INSERT INTO course_enrollments (user_id, course_id, enrolled_at) VALUES
(1, 2, '2026-04-04 11:00:00'),
(1, 7, '2026-04-04 11:05:00'),
(3, 1, '2026-04-04 11:10:00'),
(5, 4, '2026-04-04 11:15:00'),
(7, 3, '2026-04-04 11:20:00'),
(9, 5, '2026-04-04 11:25:00'),
(1, 6, '2026-04-04 11:30:00'),
(3, 8, '2026-04-04 11:35:00'),
(5, 9, '2026-04-04 11:40:00'),
(7, 10, '2026-04-04 11:45:00');

INSERT INTO lesson_progress (user_id, lesson_id, is_completed, completed_at) VALUES
(1, 3, TRUE, '2026-04-05 12:00:00'),
(1, 4, TRUE, '2026-04-05 12:10:00'),
(3, 1, TRUE, '2026-04-05 12:20:00'),
(5, 6, FALSE, NULL),
(7, 5, TRUE, '2026-04-05 12:30:00'),
(9, 7, FALSE, NULL),
(1, 8, TRUE, '2026-04-05 12:40:00'),
(3, 9, TRUE, '2026-04-05 12:50:00'),
(5, 10, FALSE, NULL),
(7, 10, TRUE, '2026-04-05 13:00:00');