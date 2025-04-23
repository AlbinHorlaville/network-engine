using GameApi.Services;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;

namespace GameApi.Controllers
{
    [ApiController]
    [Authorize]
    [Route("[controller]")]
    public class AchievementController : ControllerBase
    {
        private readonly AchievementService _achievementService;

        public AchievementController(AchievementService achievementService)
        {
            _achievementService = achievementService;
        }

        [HttpGet("details")]
        public async Task<IActionResult> GetDetailedAchievements()
        {
            var username = User.Identity?.Name!;
            var achievements = await _achievementService.GetDetailedPlayerAchievementsAsync(username);
            return achievements == null ? NotFound() : Ok(achievements);
        }
    }
}
