import cv2
import numpy as np

# 이미지를 불러옵니다
image = cv2.imread('captures/cam.jpg')  # 'your_image.jpg'를 이미지 경로로 대체하세요
gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)


# 이미지 블러 처리
gray_blurred = cv2.blur(gray, (3, 3))

# Hough 변환을 사용하여 원 검출
detected_circles = cv2.HoughCircles(gray_blurred, cv2.HOUGH_GRADIENT, 1, 20, param1=50, param2=71, minRadius=438)

# 이미지의 중심 좌표 계산
image_center = (image.shape[1] // 2, image.shape[0] // 2)

if detected_circles is not None:
    detected_circles = np.uint16(np.around(detected_circles))
    for circle in detected_circles[0, :]:
        center_x, center_y = circle[0], circle[1]

        # 원의 중심과 이미지 중심 사이의 x축 및 y축 거리 차이 계산
        distance_x = (center_x - image_center[0])
        distance_y = (center_y - image_center[1])

        # 원의 중심에 원 그리기
        cv2.circle(image, (center_x, center_y), 2, (0, 255, 0), 3)

        # 이미지의 중심에 원 그리기
        cv2.circle(image, image_center, 2, (0, 0, 255), 3)

        # 원의 중심과 이미지 중심 사이의 거리 표시
        cv2.line(image, (center_x, center_y), (image_center[0], center_y), (255, 0, 0), 2)
        cv2.line(image, (image_center[0], center_y), image_center, (255, 0, 0), 2)

        # print(f"원 중심: ({center_x}, {center_y})")
        # print(f"이미지 중심: {image_center}")
        print(f"{distance_x}/{distance_y}")

# 결과 이미지 표시
cv2.imwrite('output.jpg', image)
cv2.waitKey(0)
cv2.destroyAllWindows()

