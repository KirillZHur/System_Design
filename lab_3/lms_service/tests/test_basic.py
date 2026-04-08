import pytest


async def register_user(service_client, login, password, first_name, last_name, role):
    return await service_client.post(
        "/api/v1/auth/register",
        json={
            "login": login,
            "password": password,
            "first_name": first_name,
            "last_name": last_name,
            "role": role,
        },
    )


async def login_user(service_client, login, password):
    return await service_client.post(
        "/api/v1/auth/login",
        json={
            "login": login,
            "password": password,
        },
    )


def auth_header(token: str):
    return {"Authorization": f"Bearer {token}"}


@pytest.mark.asyncio
async def test_ping(service_client):
    response = await service_client.get("/ping")
    assert response.status == 200


@pytest.mark.asyncio
async def test_register_success(service_client):
    response = await register_user(
        service_client,
        login="teacher_test",
        password="pass123",
        first_name="Ivan",
        last_name="Petrov",
        role="teacher",
    )

    assert response.status == 201
    data = response.json()
    assert data["login"] == "teacher_test"
    assert data["first_name"] == "Ivan"
    assert data["last_name"] == "Petrov"
    assert data["role"] == "teacher"
    assert data["status"] == "registered"
    assert "id" in data


@pytest.mark.asyncio
async def test_register_bad_request(service_client):
    response = await service_client.post(
        "/api/v1/auth/register",
        json={},
    )

    assert response.status == 400


@pytest.mark.asyncio
async def test_register_conflict(service_client):
    await register_user(
        service_client,
        login="duplicate_user",
        password="pass123",
        first_name="Ivan",
        last_name="Petrov",
        role="teacher",
    )

    response = await register_user(
        service_client,
        login="duplicate_user",
        password="pass123",
        first_name="Ivan",
        last_name="Petrov",
        role="teacher",
    )

    assert response.status == 409


@pytest.mark.asyncio
async def test_login_success(service_client):
    await register_user(
        service_client,
        login="login_ok",
        password="pass123",
        first_name="Anna",
        last_name="Ivanova",
        role="student",
    )

    response = await login_user(service_client, "login_ok", "pass123")

    assert response.status == 200
    data = response.json()
    assert "token" in data
    assert data["token_type"] == "Bearer"
    assert data["login"] == "login_ok"
    assert data["role"] == "student"


@pytest.mark.asyncio
async def test_login_unauthorized(service_client):
    await register_user(
        service_client,
        login="login_fail",
        password="pass123",
        first_name="Anna",
        last_name="Ivanova",
        role="student",
    )

    response = await login_user(service_client, "login_fail", "wrong_password")
    assert response.status == 401


@pytest.mark.asyncio
async def test_get_user_by_login_success(service_client):
    await register_user(
        service_client,
        login="user_lookup",
        password="pass123",
        first_name="Kirill",
        last_name="Zhuravlev",
        role="student",
    )

    login_response = await login_user(service_client, "user_lookup", "pass123")
    token = login_response.json()["token"]

    response = await service_client.get(
        "/api/v1/users/by-login/user_lookup",
        headers=auth_header(token),
    )

    assert response.status == 200
    data = response.json()
    assert data["login"] == "user_lookup"
    assert data["first_name"] == "Kirill"


@pytest.mark.asyncio
async def test_get_user_by_login_unauthorized(service_client):
    response = await service_client.get("/api/v1/users/by-login/unknown")
    assert response.status == 400


@pytest.mark.asyncio
async def test_create_course_success(service_client):
    teacher_resp = await register_user(
        service_client,
        login="teacher_course",
        password="pass123",
        first_name="Ivan",
        last_name="Petrov",
        role="teacher",
    )
    teacher_id = teacher_resp.json()["id"]

    login_response = await login_user(service_client, "teacher_course", "pass123")
    token = login_response.json()["token"]

    response = await service_client.post(
        "/api/v1/courses",
        headers=auth_header(token),
        json={
            "title": "C++ Basics",
            "description": "Intro course",
            "teacher_id": teacher_id,
        },
    )

    assert response.status == 201
    data = response.json()
    assert data["title"] == "C++ Basics"
    assert data["teacher_id"] == teacher_id


@pytest.mark.asyncio
async def test_create_course_bad_request(service_client):
    teacher_resp = await register_user(
        service_client,
        login="teacher_bad_course",
        password="pass123",
        first_name="Ivan",
        last_name="Petrov",
        role="teacher",
    )

    login_response = await login_user(service_client, "teacher_bad_course", "pass123")
    token = login_response.json()["token"]

    response = await service_client.post(
        "/api/v1/courses",
        headers=auth_header(token),
        json={},
    )

    assert response.status == 400


@pytest.mark.asyncio
async def test_list_courses_success(service_client):
    teacher_resp = await register_user(
        service_client,
        login="teacher_list_courses",
        password="pass123",
        first_name="Ivan",
        last_name="Petrov",
        role="teacher",
    )
    teacher_id = teacher_resp.json()["id"]

    login_response = await login_user(service_client, "teacher_list_courses", "pass123")
    token = login_response.json()["token"]

    await service_client.post(
        "/api/v1/courses",
        headers=auth_header(token),
        json={
            "title": "Algorithms",
            "description": "Base course",
            "teacher_id": teacher_id,
        },
    )

    response = await service_client.get(
        "/api/v1/courses",
        headers=auth_header(token),
    )

    assert response.status == 200
    data = response.json()
    assert isinstance(data, list)
    assert len(data) >= 1


@pytest.mark.asyncio
async def test_add_lesson_success(service_client):
    teacher_resp = await register_user(
        service_client,
        login="teacher_lesson",
        password="pass123",
        first_name="Ivan",
        last_name="Petrov",
        role="teacher",
    )
    teacher_id = teacher_resp.json()["id"]

    login_response = await login_user(service_client, "teacher_lesson", "pass123")
    token = login_response.json()["token"]

    course_response = await service_client.post(
        "/api/v1/courses",
        headers=auth_header(token),
        json={
            "title": "Databases",
            "description": "DB course",
            "teacher_id": teacher_id,
        },
    )
    course_id = course_response.json()["id"]

    response = await service_client.post(
        f"/api/v1/courses/{course_id}/lessons",
        headers=auth_header(token),
        json={
            "title": "Lesson 1",
            "content": "Intro to DB",
        },
    )

    assert response.status == 201
    data = response.json()
    assert data["course_id"] == course_id
    assert data["title"] == "Lesson 1"


@pytest.mark.asyncio
async def test_add_lesson_bad_request(service_client):
    teacher_resp = await register_user(
        service_client,
        login="teacher_lesson_bad",
        password="pass123",
        first_name="Ivan",
        last_name="Petrov",
        role="teacher",
    )
    teacher_id = teacher_resp.json()["id"]

    login_response = await login_user(service_client, "teacher_lesson_bad", "pass123")
    token = login_response.json()["token"]

    course_response = await service_client.post(
        "/api/v1/courses",
        headers=auth_header(token),
        json={
            "title": "Networks",
            "description": "Network course",
            "teacher_id": teacher_id,
        },
    )
    course_id = course_response.json()["id"]

    response = await service_client.post(
        f"/api/v1/courses/{course_id}/lessons",
        headers=auth_header(token),
        json={},
    )

    assert response.status == 400


@pytest.mark.asyncio
async def test_list_lessons_success(service_client):
    teacher_resp = await register_user(
        service_client,
        login="teacher_list_lessons",
        password="pass123",
        first_name="Ivan",
        last_name="Petrov",
        role="teacher",
    )
    teacher_id = teacher_resp.json()["id"]

    login_response = await login_user(service_client, "teacher_list_lessons", "pass123")
    token = login_response.json()["token"]

    course_response = await service_client.post(
        "/api/v1/courses",
        headers=auth_header(token),
        json={
            "title": "OS",
            "description": "Operating systems",
            "teacher_id": teacher_id,
        },
    )
    course_id = course_response.json()["id"]

    await service_client.post(
        f"/api/v1/courses/{course_id}/lessons",
        headers=auth_header(token),
        json={
            "title": "Lesson 1",
            "content": "Processes",
        },
    )

    response = await service_client.get(
        f"/api/v1/courses/{course_id}/lessons",
        headers=auth_header(token),
    )

    assert response.status == 200
    data = response.json()
    assert isinstance(data, list)
    assert len(data) >= 1


@pytest.mark.asyncio
async def test_enroll_user_success(service_client):
    teacher_resp = await register_user(
        service_client,
        login="teacher_enroll",
        password="pass123",
        first_name="Ivan",
        last_name="Petrov",
        role="teacher",
    )
    teacher_id = teacher_resp.json()["id"]

    student_resp = await register_user(
        service_client,
        login="student_enroll",
        password="pass123",
        first_name="Anna",
        last_name="Ivanova",
        role="student",
    )
    student_id = student_resp.json()["id"]

    teacher_login = await login_user(service_client, "teacher_enroll", "pass123")
    teacher_token = teacher_login.json()["token"]

    student_login = await login_user(service_client, "student_enroll", "pass123")
    student_token = student_login.json()["token"]

    course_response = await service_client.post(
        "/api/v1/courses",
        headers=auth_header(teacher_token),
        json={
            "title": "Math",
            "description": "Math course",
            "teacher_id": teacher_id,
        },
    )
    course_id = course_response.json()["id"]

    response = await service_client.post(
        f"/api/v1/courses/{course_id}/enrollments",
        headers=auth_header(student_token),
        json={"user_id": student_id},
    )

    assert response.status == 200
    data = response.json()
    assert data["status"] == "enrolled"
    assert data["user_id"] == student_id
    assert data["course_id"] == course_id


@pytest.mark.asyncio
async def test_enroll_user_bad_request(service_client):
    teacher_resp = await register_user(
        service_client,
        login="teacher_enroll_bad",
        password="pass123",
        first_name="Ivan",
        last_name="Petrov",
        role="teacher",
    )
    teacher_id = teacher_resp.json()["id"]

    student_resp = await register_user(
        service_client,
        login="student_enroll_bad",
        password="pass123",
        first_name="Anna",
        last_name="Ivanova",
        role="student",
    )

    teacher_login = await login_user(service_client, "teacher_enroll_bad", "pass123")
    teacher_token = teacher_login.json()["token"]

    student_login = await login_user(service_client, "student_enroll_bad", "pass123")
    student_token = student_login.json()["token"]

    course_response = await service_client.post(
        "/api/v1/courses",
        headers=auth_header(teacher_token),
        json={
            "title": "Physics",
            "description": "Physics course",
            "teacher_id": teacher_id,
        },
    )
    course_id = course_response.json()["id"]

    response = await service_client.post(
        f"/api/v1/courses/{course_id}/enrollments",
        headers=auth_header(student_token),
        json={},
    )

    assert response.status == 400


@pytest.mark.asyncio
async def test_get_user_courses_success(service_client):
    teacher_resp = await register_user(
        service_client,
        login="teacher_user_courses",
        password="pass123",
        first_name="Ivan",
        last_name="Petrov",
        role="teacher",
    )
    teacher_id = teacher_resp.json()["id"]

    student_resp = await register_user(
        service_client,
        login="student_user_courses",
        password="pass123",
        first_name="Anna",
        last_name="Ivanova",
        role="student",
    )
    student_id = student_resp.json()["id"]

    teacher_login = await login_user(service_client, "teacher_user_courses", "pass123")
    teacher_token = teacher_login.json()["token"]

    student_login = await login_user(service_client, "student_user_courses", "pass123")
    student_token = student_login.json()["token"]

    course_response = await service_client.post(
        "/api/v1/courses",
        headers=auth_header(teacher_token),
        json={
            "title": "Programming",
            "description": "Programming course",
            "teacher_id": teacher_id,
        },
    )
    course_id = course_response.json()["id"]

    await service_client.post(
        f"/api/v1/courses/{course_id}/enrollments",
        headers=auth_header(student_token),
        json={"user_id": student_id},
    )

    response = await service_client.get(
        f"/api/v1/users/{student_id}/courses",
        headers=auth_header(student_token),
    )

    assert response.status == 200
    data = response.json()
    assert isinstance(data, list)
    assert len(data) >= 1


@pytest.mark.asyncio
async def test_mark_progress_success(service_client):
    teacher_resp = await register_user(
        service_client,
        login="teacher_progress",
        password="pass123",
        first_name="Ivan",
        last_name="Petrov",
        role="teacher",
    )
    teacher_id = teacher_resp.json()["id"]

    student_resp = await register_user(
        service_client,
        login="student_progress",
        password="pass123",
        first_name="Anna",
        last_name="Ivanova",
        role="student",
    )
    student_id = student_resp.json()["id"]

    teacher_login = await login_user(service_client, "teacher_progress", "pass123")
    teacher_token = teacher_login.json()["token"]

    student_login = await login_user(service_client, "student_progress", "pass123")
    student_token = student_login.json()["token"]

    course_response = await service_client.post(
        "/api/v1/courses",
        headers=auth_header(teacher_token),
        json={
            "title": "ML",
            "description": "Machine Learning",
            "teacher_id": teacher_id,
        },
    )
    course_id = course_response.json()["id"]

    lesson_response = await service_client.post(
        f"/api/v1/courses/{course_id}/lessons",
        headers=auth_header(teacher_token),
        json={
            "title": "Lesson 1",
            "content": "Introduction",
        },
    )
    lesson_id = lesson_response.json()["id"]

    await service_client.post(
        f"/api/v1/courses/{course_id}/enrollments",
        headers=auth_header(student_token),
        json={"user_id": student_id},
    )

    response = await service_client.post(
        f"/api/v1/lessons/{lesson_id}/progress",
        headers=auth_header(student_token),
        json={"user_id": student_id, "completed": True},
    )

    assert response.status == 200
    data = response.json()
    assert data["status"] == "completed"
    assert data["user_id"] == student_id
    assert data["lesson_id"] == lesson_id
    assert data["completed"] is True


@pytest.mark.asyncio
async def test_mark_progress_bad_request(service_client):
    teacher_resp = await register_user(
        service_client,
        login="teacher_progress_bad",
        password="pass123",
        first_name="Ivan",
        last_name="Petrov",
        role="teacher",
    )
    teacher_id = teacher_resp.json()["id"]

    student_resp = await register_user(
        service_client,
        login="student_progress_bad",
        password="pass123",
        first_name="Anna",
        last_name="Ivanova",
        role="student",
    )
    student_id = student_resp.json()["id"]

    teacher_login = await login_user(service_client, "teacher_progress_bad", "pass123")
    teacher_token = teacher_login.json()["token"]

    student_login = await login_user(service_client, "student_progress_bad", "pass123")
    student_token = student_login.json()["token"]

    course_response = await service_client.post(
        "/api/v1/courses",
        headers=auth_header(teacher_token),
        json={
            "title": "AI",
            "description": "Artificial Intelligence",
            "teacher_id": teacher_id,
        },
    )
    course_id = course_response.json()["id"]

    lesson_response = await service_client.post(
        f"/api/v1/courses/{course_id}/lessons",
        headers=auth_header(teacher_token),
        json={
            "title": "Lesson 1",
            "content": "Intro",
        },
    )
    lesson_id = lesson_response.json()["id"]

    await service_client.post(
        f"/api/v1/courses/{course_id}/enrollments",
        headers=auth_header(student_token),
        json={"user_id": student_id},
    )

    response = await service_client.post(
        f"/api/v1/lessons/{lesson_id}/progress",
        headers=auth_header(student_token),
        json={"user_id": student_id, "completed": False},
    )

    assert response.status == 400