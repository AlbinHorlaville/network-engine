using GameApi.Services;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;

namespace GameApi.Controllers
{
    [ApiController]
    [Authorize]
    [Route("[controller]")]
    public class AchievementsController : ControllerBase
    {
        private readonly AchievementService _achievementService;

        public AchievementsController(AchievementService achievementService)
        {
            _achievementService = achievementService;
        }

        [HttpGet]
        public async Task<IActionResult> Get()
        {
            var username = User.Identity?.Name!;
            var achievements = await _achievementService.GetPlayerAchievementsAsync(username);
            return Ok(achievements);
        }
    }
}
