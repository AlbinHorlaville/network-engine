using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;

namespace GameApi.Controllers
{
    [Authorize]
    [ApiController]
    [Route("achievements")]
    public class AchievementsController : ControllerBase
    {
        [HttpGet]
        public IActionResult GetAchievements()
        {
            var user = User.Identity.Name!;
            var stat = StatsController.stats.GetValueOrDefault(user, new PlayerStats());
            var achievements = new List<string>();

            if (stat.GamesWon >= 5)
                achievements.Add("Gagné 5 parties !");
            if (stat.CubesCleared >= 50)
                achievements.Add("Expulsé 50 cubes !");

            return Ok(achievements);
        }
    }
}
