import json
import requests
from bs4 import BeautifulSoup
from selenium import webdriver
from time import sleep


def get_scores():
    live_scores_url = "https://www.espncricinfo.com/live-cricket-score"
    print('1')
    options = webdriver.ChromeOptions()
    options.add_argument("--headless=new")
    options.add_argument("--start-maximized")
    driver = webdriver.Remote(
        command_executor="http://selenium_chrome:4444",
        options=options,
    )

    print('2')
    driver.get(live_scores_url)

    print('3')
    sleep(2)

    print('4')
    live_scores_soup = BeautifulSoup(driver.page_source, "lxml")

    score_boxes = live_scores_soup.find_all("div", class_="ds-p-0")[
        0
    ].find_all("div", class_="ds-text-compact-xxs")

    details = []
    for box in score_boxes:
        match_status = box.find("div", class_="ds-truncate").find("span").find("span").text.capitalize()
        match_details = box.find("div", class_="ds-truncate").find(
            "div", class_="ds-truncate"
        ).text

        teams = box.find_all("div", class_="ds-my-1")

        team_1_name = teams[0].find("div", class_="ds-mr-1").text

        if teams[0].find("div", class_="ds-text-right") is not None:
            team_1_score = (
                teams[0]
                .find("div", class_="ds-text-right")
                .text.replace("\u00a0", " ")
            )

        else:
            team_1_score = "Yet to bat"

        team_2_name = teams[1].find("div", class_="ds-mr-1").text
        if teams[1].find("div", class_="ds-text-right") is not None:
            team_2_score = (
                teams[1]
                .find("div", class_="ds-text-right")
                .text.replace("\u00a0", " ")
            )
        else:
            team_2_score = "Yet to bat"

        match_details = {
            "match_status": match_status,
            "match_details": match_details,
            "team_1_name": team_1_name,
            "team_1_score": team_1_score,
            "team_2_name": team_2_name,
            "team_2_score": team_2_score,
        }
        details.append(match_details)
    driver.quit()

    for num, match_details in enumerate(details):
        print(f'JsonObject obj{num+1} = doc.add<JsonObject>();')
        print(f'obj{num+1}["match_status"] = "{match_details["match_status"]}";')
        print(f'obj{num+1}["match_details"] = "{match_details["match_details"]}";')
        print(f'obj{num+1}["team_1_name"] = "{match_details["team_1_name"]}";')
        print(f'obj{num+1}["team_1_score"] = "{match_details["team_1_score"]}";')
        print(f'obj{num+1}["team_2_name"] = "{match_details["team_2_name"]}";')
        print(f'obj{num+1}["team_2_score"] = "{match_details["team_2_score"]}";')

    return json.dumps(details)


if __name__ == "__main__":
    deets = get_scores()
    print(deets)
