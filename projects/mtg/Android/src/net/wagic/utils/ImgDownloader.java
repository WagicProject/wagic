package net.wagic.utils;

import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;
import org.jsoup.nodes.Node;

import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;
import java.util.Enumeration;

import net.lingala.zip4j.model.ZipParameters;
import net.lingala.zip4j.model.enums.CompressionMethod;

import java.io.*;
import java.net.URL;
import java.net.HttpURLConnection;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Map;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.HashMap;
import java.util.stream.Stream;

import android.graphics.*;
import android.app.ProgressDialog;
import org.libsdl.app.SDLActivity;

public class ImgDownloader {

    private static String convertStreamToString(java.io.InputStream inputStream) {
        final int bufferSize = 1024;
        final char[] buffer = new char[bufferSize];
        final StringBuilder out = new StringBuilder();
        try {
            Reader in = new InputStreamReader(inputStream, StandardCharsets.ISO_8859_1);
            for (; ; ) {
                int rsz = in.read(buffer, 0, buffer.length);
                if (rsz < 0)
                    break;
                out.append(buffer, 0, rsz);
            }
        } catch (Exception e) {
        }
        return out.toString();
    }

    private static String readLineByLineJava8(String filePath) {
        StringBuilder contentBuilder = new StringBuilder();

        try {
            File file = new File(filePath);
            BufferedReader br = new BufferedReader(new FileReader(file));

            String st;
            while ((st = br.readLine()) != null)
                contentBuilder.append(st).append("\n");
        } catch (Exception e) {
            e.printStackTrace();
        }

        return contentBuilder.toString();
    }

    public static String getSetInfo(String setName, boolean zipped, String path) {
        String cardsfilepath = "";
        boolean todelete = false;
        if (zipped) {
            File resFolder = new File(path + File.separator);
            File[] listOfFile = resFolder.listFiles();
            ZipFile zipFile = null;
            InputStream stream = null;
            java.nio.file.Path filePath = null;
            try {
                for (int i = 0; i < listOfFile.length; i++) {
                    if (listOfFile[i].getName().contains(".zip")) {
                        zipFile = new ZipFile(path + File.separator + listOfFile[i].getName());
                        break;
                    }
                }
                if (zipFile == null)
                    return "";
                Enumeration<? extends ZipEntry> e = zipFile.entries();
                while (e.hasMoreElements()) {
                    ZipEntry entry = e.nextElement();
                    String entryName = entry.getName();
                    if (entryName.contains("sets" + File.separator)) {
                        if (entryName.contains("_cards.dat")) {
                            String[] names = entryName.split(File.separator);
                            if (setName.equalsIgnoreCase(names[1])) {
                                stream = zipFile.getInputStream(entry);
                                byte[] buffer = new byte[1];
                                java.nio.file.Path outDir = Paths.get(path + File.separator);
                                filePath = outDir.resolve("_cards.dat");
                                try {
                                    FileOutputStream fos = new FileOutputStream(filePath.toFile());
                                    BufferedOutputStream bos = new BufferedOutputStream(fos, buffer.length);
                                    int len;
                                    while ((len = stream.read(buffer)) != -1) {
                                        bos.write(buffer, 0, len);
                                    }
                                    fos.close();
                                    bos.close();
                                    cardsfilepath = filePath.toString();
                                    todelete = true;
                                } catch (Exception ex) {
                                }
                                break;
                            }
                        }
                    }
                }
            } catch (IOException ioe) {
            } finally {
                try {
                    if (zipFile != null) {
                        zipFile.close();
                    }
                } catch (IOException ioe) {
                }
            }
        } else {
            File setFolder = new File(path + File.separator + "sets" + File.separator + setName + File.separator);
            cardsfilepath = setFolder.getAbsolutePath() + File.separator + "_cards.dat";
        }
        String lines = readLineByLineJava8(cardsfilepath);
        if (todelete) {
            File del = new File(cardsfilepath);
            del.delete();
        }
        int totalcards = 0;
        String findStr = "total=";
        int lastIndex = lines.indexOf(findStr);
        String totals = lines.substring(lastIndex, lines.indexOf("\n", lastIndex));
        totalcards = Integer.parseInt(totals.split("=")[1]);
        findStr = "name=";
        lastIndex = lines.indexOf(findStr);
        String name = lines.substring(lastIndex, lines.indexOf("\n", lastIndex)).split("=")[1];
        return name + " (" + totalcards + " cards)";
    }

    final static int[] tokenids = new int[]{1, 2, 3, 6, 10, 14, 15, 16, 17, 18, 19, 20, 21, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 37, 38, 39, 40, 41, 42, 43, 46, 47, 48, 49, 50, 51, 52, 54, 55, 56, 57, 58,
            59, 60, 61, 62, 63, 64, 65, 66, 67, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 80, 81, 82, 83, 84, 85, 87, 88, 89, 90, 91, 92, 93, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 113, 114, 115, 116,
            117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 143, 144, 145, 146, 147, 148, 149, 150, 151, 166, 167, 168, 169, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 185, 186, 187,
            188, 189, 190, 191, 200, 202, 203, 211, 212, 213, 218, 219, 220, 221, 222, 223, 234, 235, 236, 237, 238, 239, 249, 250, 251, 252, 253, 254, 261, 262, 263, 264, 265, 272, 273, 274, 275, 276, 277,
            278, 279, 280, 281, 282, 288, 289, 291, 292, 293, 294, 295, 314, 315, 316, 317, 318, 319, 324, 325, 328, 329, 330, 331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 349, 350, 351, 352, 353, 354,
            355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 382, 384, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397,
            398, 399, 400, 401, 402, 403, 404, 405, 450, 451, 452, 453, 454, 455, 456, 457, 458, 459, 460, 465, 466, 467, 468, 469, 470, 471, 472, 473, 474, 475, 476, 477, 478, 479, 480, 481, 482, 483, 484,
            485, 486, 487, 488, 489, 490, 510, 511, 512, 513, 533, 534, 535, 538, 539, 540, 541, 542, 543, 544, 796, 797, 798, 799, 800, 801, 802, 803, 804, 805, 806, 807, 808, 809, 810, 811, 812, 813, 814,
            815, 816, 817, 818, 819, 820, 841, 842, 843, 844, 845, 846, 848, 849, 886, 888, 890, 939, 940, 941, 942, 943, 944, 946, 947, 948, 949, 950, 970, 972, 973, 977, 1539, 1995, 1996, 1997, 1998, 1999,
            2000, 2001, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2025, 2027, 2028, 2029, 2031, 2032, 2033, 2034, 2035, 2037, 2038, 2039, 2040, 2041, 2042, 2043,
            2045, 2046, 2047, 2049, 2050, 2051, 2052, 2053, 2055, 2057, 2058, 2059, 2060, 2061, 2062, 2063, 2064, 2065, 2066, 2068, 2069, 2070, 2071, 2072, 2102, 2156, 2158, 2159, 2162, 2163, 2164, 2165,
            2166, 2167, 2168, 2170, 2171, 2172, 2174, 2175, 2177, 2178, 2179, 2180, 2181, 2182, 2183, 2184, 2185, 2186, 2187, 2188, 2189, 2190, 2197, 2235, 2236, 2238, 2239, 2240, 2406, 2407, 2409, 2410,
            2411, 2412, 2413, 2414, 2415, 2416, 2426, 2427, 2428, 2429, 2430, 2431, 2433, 2434, 2436, 2439, 2453, 2454, 2455, 2456, 2457, 2458, 2459, 2460, 2462, 2463, 2474, 2475, 2476, 2477, 2479, 2480,
            2481, 2482, 2483, 2484, 2493, 2494, 2495, 2498, 2499, 2500, 2501, 2502, 2503, 2504, 2515, 2516, 2517, 2518, 2519, 2521, 2522, 2523, 2524, 2525, 2535, 2537, 2538, 2539, 2540, 2541, 2542, 2543,
            2544, 2545, 2557, 2558, 2560, 2561, 2562, 2563, 2564, 2565, 2566, 2567, 2580, 2581, 2582, 2583, 2584, 2585, 2586, 2587, 2588, 2589, 2599, 2600, 2601, 2605, 2606, 2607, 2608, 2609, 2610, 2611,
            2620, 2621, 2622, 2623, 2626, 2628, 2629, 2632, 2633, 2635, 2703, 2754, 2756, 2757, 2758, 2760, 2762, 2766, 2767, 2768, 2770, 2771, 2772, 2774, 2775, 2776, 2778, 2779, 2780, 2781, 2782, 2783,
            2784, 2785, 2786, 2787, 2788, 2789, 2790, 2791, 2792, 2794, 2795, 2796, 2797, 2798, 2799, 2800, 2801, 2802, 2803, 2804, 2805, 2806, 2807, 2808, 2810, 2812, 2813, 2814, 2815, 2816, 2817, 2818,
            2819, 2823, 2824, 2825, 2826, 2827, 2828, 2829, 2830, 2831, 2832, 2833, 2834, 2835, 2836, 2837, 2838, 2839, 2840, 2841, 2842, 2843, 2844, 2845, 2846, 2852, 2853, 2854, 2855, 2856, 2857, 2858,
            2859, 2860, 2861, 2862, 2863, 2864, 2870, 2871, 2872, 2873, 2874, 2875, 2876, 3134, 3135, 3136, 3137, 3138, 3139, 3140, 3141, 3142, 3144, 3146, 3147, 3148, 3149, 3150, 3151, 3152, 3153, 3154,
            3155, 3160, 3162, 3163, 3164, 3165, 3166, 3167, 3169, 3170, 3171, 3435, 3436, 3437, 3438, 3439, 3440, 3441, 3442, 3444, 3445, 3446, 3447, 3448, 3449, 3450, 3452, 3453, 3457, 3458, 3459, 3460,
            3461, 3462, 3463, 3464, 3466, 3467, 3468, 3469, 3470, 3471, 3473, 3474, 3475, 3479, 3480, 3481, 3482, 3483, 3484, 3485, 3486, 3488, 3489, 3490, 3491, 3492, 3493, 3494, 3495, 3496, 3500, 3501,
            3502, 3503, 3504, 3505, 3506, 3508, 3510, 3511, 3513, 3514, 3515, 3516, 3517, 3518, 3519, 3523, 3524, 3525, 3526, 3527, 3528, 3529, 3530, 3532, 3533, 3534, 3535, 3536, 3537, 3538, 3539, 3540,
            3544, 3545, 3546, 3547, 3548, 3549, 3550, 3551, 3553, 3554, 3555, 3556, 3557, 3558, 3559, 3560, 3561, 3565, 3566, 3567, 3568, 3569, 3571, 3572, 3573, 3575, 3576, 3577, 3578, 3579, 3581, 3582,
            3583, 3584, 3589, 3590, 3591, 3592, 3593, 3594, 3595, 3596, 3598, 3599, 3600, 3601, 3602, 3603, 3604, 3605, 3606, 3610, 3611, 3612, 3614, 3615, 3616, 3617, 3618, 3619, 3620, 3621, 3623, 3624,
            3625, 3626, 3627, 3629, 3631, 3633, 3639, 3640, 3642, 3643, 3644, 3645, 3647, 3648, 3649, 3650, 3651, 3652, 3653, 3654, 3655, 3656, 3657, 3659, 3660, 3661, 3662, 3663, 3666, 3668, 3669, 3670,
            3671, 3672, 3673, 3675, 3676, 3677, 3678, 3679, 3680, 3681, 3682, 3683, 3684, 3685, 3688, 3689, 3690, 3691, 3694, 3695, 3696, 3697, 3698, 3699, 3700, 3701, 3702, 3703, 3704, 3705, 3706, 3708,
            3709, 3710, 3711, 3713, 3714, 3715, 3717, 3718, 3721, 3723, 3724, 3725, 3726, 3727, 3728, 3729, 3730, 3732, 3733, 3734, 3735, 3736, 3737, 3738, 3739, 3740, 3741, 3742, 3743, 3744, 3747, 3748,
            3749, 3750, 3751, 3752, 3753, 3754, 3755, 3756, 3757, 3758, 3759, 3760, 3761, 3762, 3763, 3764, 3765, 3766, 3767, 3768, 3771, 3772, 3773, 3774, 3775, 3776, 3777, 3778, 3779, 3780, 3781, 3782,
            3783, 3784, 3785, 3786, 3787, 3788, 3789, 3790, 3791, 3792, 3992, 3997, 4001, 4005, 4009, 4014, 4018, 4022, 4026, 4030, 4034, 4082, 4083, 4085, 4086, 4087, 4088, 4089, 4090, 4091, 4092, 4093,
            4094, 4095, 4096, 4097, 4098, 4099, 4100, 4101, 4105, 4106, 4107, 4108, 4109, 4110, 4111, 4112, 4113, 4114, 4115, 4116, 4117, 4118, 4119, 4120, 4121, 4122, 4123, 4126, 4127, 4128, 4129, 4130,
            4131, 4132, 4133, 4134, 4135, 4136, 4137, 4138, 4139, 4140, 4141, 4142, 4143, 4144, 4150, 4151, 4152, 4153, 4154, 4155, 4156, 4157, 4158, 4159, 4160, 4162, 4163, 4164, 4166, 4167, 4168, 4169,
            4170, 4171, 4172, 4173, 4174, 4176, 4177, 4178, 4179, 4180, 4181, 4182, 4183, 4184, 4185, 4186, 4188, 4189, 4190, 4191, 4192, 4193, 4194, 4195, 4196, 4197, 4198, 4199, 4201, 4202, 4203, 4204,
            4205, 4206, 4207, 4208, 4209, 4210, 4211, 4213, 4214, 4215, 4216, 4217, 4218, 4219, 4220, 4222, 4223, 4224, 4225, 4227, 4228, 4229, 4230, 4231, 4232, 4233, 4234, 4235, 4236, 4237, 4239, 4240,
            4241, 4242, 4243, 4244, 4245, 4246, 4247, 4248, 4249, 4250, 4252, 4254, 4255, 4256, 4257, 4258, 4259, 4260, 4261, 4262, 4263, 4265, 4266, 4267, 4268, 4269, 4270, 4271, 4272, 4273, 4275, 4277,
            4278, 4280, 4281, 4282, 4283, 4284, 4285, 4286, 4287, 4288, 4289, 4291, 4293, 4294, 4295, 4296, 4297, 4298, 4299, 4300, 4301, 4302, 4303, 4304, 4306, 4307, 4308, 4309, 4310, 4311, 4312, 4313,
            4314, 4315, 4316, 4318, 4319, 4320, 4321, 4322, 4323, 4324, 4325, 4326, 4327, 4328, 4329, 4331, 4332, 4333, 4334, 4335, 4336, 4337, 4338, 4339, 4340, 4341, 4343, 4344, 4345, 4346, 4347, 4348,
            4349, 4350, 4351, 4352, 4353, 4354, 4356, 4357, 4358, 4359, 4360, 4361, 4362, 4363, 4364, 4365, 4366, 4368, 4369, 4370, 4371, 4372, 4373, 4375, 4376, 4377, 4378, 4379, 4380, 4382, 4383, 4384,
            4385, 4386, 4387, 4388, 4389, 4390, 4391, 4392, 4394, 4395, 4396, 4397, 4398, 4399, 4400, 4401, 4402, 4403, 4404, 4405, 4407, 4408, 4409, 4410, 4411, 4412, 4413, 4414, 4415, 4416, 4417, 4419,
            4420, 4421, 4422, 4423, 4424, 4425, 4426, 4427, 4429, 4430, 4431, 4446, 4447, 4448, 4449, 4450, 4451, 4452, 4454, 4455, 4456, 4457, 4458, 4459, 4460, 4461, 4462, 4463, 4464, 4465, 4466, 4467,
            4468, 4469, 4470, 4471, 4472, 4476, 4489, 4490, 4492, 4493, 4494, 4496, 4497, 4498, 4499, 4500, 4501, 4502, 4503, 4504, 4505, 4506, 4507, 4508, 4509, 4510, 4511, 4513, 4514, 4515, 4516, 4517,
            4518, 4519, 4520, 4521, 4522, 4523, 4524, 4525, 4526, 4527, 4528, 4529, 4530, 4531, 4533, 4534, 4535, 4536, 4537, 4538, 4539, 4540, 4541, 4542, 4543, 4544, 4545, 4546, 4547, 4548, 4549, 4550,
            4552, 4553, 4554, 4555, 4556, 4557, 4558, 4559, 4560, 4561, 4562, 4563, 4564, 4565, 4566, 4567, 4568, 4570, 4573, 4574, 4575, 4576, 4577, 4578, 4579, 4580, 4581, 4582, 4583, 4584, 4585, 4586,
            4587, 4588, 4589, 4590, 4592, 4593, 4594, 4595, 4596, 4597, 4599, 4600, 4601, 4602, 4603, 4604, 4605, 4606, 4607, 4608, 4609, 4610, 4611, 4612, 4613, 4614, 4615, 4616, 4617, 4618, 4619, 4620,
            4621, 4622, 4623, 4624, 4625, 4626, 4627, 4628, 4629, 4630, 4995, 5000, 5003, 5006, 5009, 5010, 5014, 5015, 5017, 5019, 5023, 5026, 5029, 5038, 5041, 5043, 5044, 5050, 5054, 5056, 5063, 5067,
            5073, 5080, 5081, 5083, 5087, 5091, 5093, 5095, 5098, 5102, 5103, 5105, 5107, 5108, 5109, 5112, 5114, 5115, 5116, 5117, 5118, 5119, 5120, 5121, 5122, 5123, 5124, 5125, 5126, 5127, 5128, 5129,
            5131, 5132, 5133, 5135, 5136, 5137, 5138, 5139, 5140, 5141, 5143, 5144, 5145, 5146, 5147, 5148, 5149, 5150, 5151, 5152, 5154, 5155, 5156, 5158, 5159, 5160, 5161, 5162, 5163, 5164, 5166, 5167,
            5168, 5169, 5170, 5171, 5172, 5177, 5178, 5179, 5180, 5181, 5182, 5187, 5188, 5189, 5190, 5191, 5192, 5197, 5198, 5199, 5200, 5201, 5202, 5207, 5208, 5209, 5210, 5211, 5212, 5217, 5218, 5219,
            5220, 5221, 5222, 5227, 5228, 5229, 5230, 5231, 5232, 5238, 5239, 5240, 5241, 5242, 5243, 5248, 5249, 5250, 5251, 5252, 5253, 5258, 5259, 5260, 5261, 5262, 5263, 5269, 5270, 5271, 5272, 5273,
            5275, 5283, 5284, 5285, 5291, 5292, 5293, 5299, 5300, 5301, 5307, 5308, 5309, 5315, 5316, 5317, 5323, 5324, 5325, 5331, 5332, 5333, 5339, 5340, 5341, 5347, 5348, 5349, 5355, 5356, 5357, 5363,
            5364, 5365, 5372, 5373, 5374, 5375, 5376, 5377, 5437, 5438, 5439, 5440, 5441, 5442, 5443, 5446, 5447, 5448, 5449, 5450, 5451, 5452, 5453, 5454, 5455, 5456, 5457, 5458, 5459, 5460, 5461, 5462,
            5470, 5471, 5472, 5476, 5477, 5478, 5482, 5483, 5484, 5488, 5489, 5490, 5495, 5496, 5500, 5501, 5502, 5506, 5507, 5508, 5512, 5513, 5514, 5518, 5520, 5521, 5532, 5533, 5534, 5535, 5536, 5537,
            5538, 5539, 5593, 5594, 5598, 5599, 5600, 5604, 5605, 5606, 5610, 5611, 5612, 5616, 5618, 5619, 5635, 5636, 5637, 5638, 5639, 5640, 5641, 5642, 5643, 5644, 5645, 5646, 5647, 5649, 5650, 5651,
            5652, 5653, 5654, 5655, 5656, 5657, 5658, 5659, 5660, 5661, 5662, 5665, 5666, 5667, 5668, 5669, 5671, 5672, 5673, 5674, 5675, 5676, 5677, 5678, 5679, 5681, 5691, 5692, 5693, 5694, 5695, 5696,
            5697, 5698, 5699, 5700, 5701, 5702, 5703, 5705, 5706, 5707, 5708, 5709, 5718, 5748, 5749, 5750, 5751, 5752, 5753, 5754, 5755, 5756, 5758, 5759, 5760, 5764, 5765, 5766, 5767, 5768, 5769, 5770,
            5771, 5772, 5774, 5775, 5776, 5780, 5781, 5782, 5783, 5784, 5785, 5786, 5787, 5788, 5790, 5791, 5792, 5796, 5797, 5798, 5799, 5800, 5801, 5802, 5806, 5807, 5810, 5811, 5812, 5816, 5817, 5818,
            5819, 5820, 5821, 5822, 5824, 5825, 5827, 5828, 5829, 5833, 5834, 5835, 5836, 5837, 5838, 5839, 5840, 5841, 5843, 5844, 5845, 5849, 5850, 5851, 5852, 5853, 5854, 5855, 5856, 5857, 5859, 5860,
            5861, 5865, 5866, 5868, 5869, 5870, 5871, 5872, 5873, 5874, 5876, 5877, 5878, 5882, 5883, 5884, 5885, 5886, 5887, 5888, 5889, 5890, 5892, 5893, 5894, 5898, 5899, 5900, 5901, 5902, 5903, 5904,
            5905, 5906, 5908, 5909, 5910, 5914, 5915, 5916, 5917, 5918, 5919, 5920, 5921, 5922, 5924, 5925, 5926, 5936, 5937, 5938, 5939, 5940, 6015, 6018, 6019, 6020, 6021, 6022, 6023, 6024, 6025, 6026,
            6027, 6028, 6036, 6037, 6038, 6039, 6040, 6041, 6042, 6043, 6045, 6046, 6047, 6048, 6053, 6055, 6057, 6059, 6060, 6062, 6065, 6066, 6068, 6070, 6072, 6074, 6076, 6078, 6081, 6083, 6085, 6241,
            6242, 6243, 6245, 6246, 6249, 6251, 6252, 6253, 6254, 6255, 6256, 6257, 6258, 6267, 6268, 6269, 6271, 6272, 6275, 6277, 6278, 6279, 6280, 6281, 6282, 6283, 6284, 6293, 6294, 6295, 6297, 6298,
            6301, 6303, 6304, 6305, 6306, 6307, 6308, 6309, 6310, 6319, 6320, 6321, 6323, 6324, 6327, 6329, 6330, 6331, 6332, 6333, 6334, 6335, 6336, 6345, 6346, 6347, 6349, 6350, 6353, 6355, 6356, 6357,
            6358, 6359, 6360, 6361, 6362, 6371, 6372, 6373, 6375, 6376, 6379, 6381, 6382, 6383, 6384, 6385, 6386, 6387, 6388, 6397, 6398, 6399, 6401, 6402, 6405, 6407, 6408, 6409, 6410, 6411, 6412, 6413,
            6414, 6432, 6433, 6434, 6435, 6436, 6437, 6440, 6441, 6442, 6443, 6444, 6445, 6448, 6449, 6450, 6451, 6452, 6453, 6456, 6457, 6458, 6459, 6460, 6461, 6464, 6465, 6466, 6467, 6468, 6469, 6472,
            6473, 6474, 6475, 6476, 6477, 6480, 6481, 6482, 6483, 6484, 6485, 6488, 6489, 6490, 6491, 6492, 6493, 6496, 6497, 6498, 6499, 6500, 6501, 6504, 6505, 6506, 6507, 6508, 6509, 6512, 6513, 6514,
            6515, 6516, 6517, 6540, 6542, 6543, 6544, 6545, 6546, 6547, 6548, 6549};

    public static String getSpecialCardUrl(String id) {
        String cardurl = "";

        if (id.equals("15208711"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/c/9c138bf9-8be6-4f1a-a82c-a84938ab84f5.jpg?1562279137";
        else if (id.equals("15208712"))
            cardurl = "https://img.scryfall.com/cards/normal/front/d/4/d453ee89-6122-4d51-989c-e78b046a9de3.jpg?1561758141";
        else if (id.equals("2050321"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/8/18b9c83d-4422-4b95-9fc2-070ed6b5bdf6.jpg?1562701921";
        else if (id.equals("2050322"))
            cardurl = "https://deckmaster.info/images/cards/M11/-239-hr.jpg";
        else if (id.equals("22010012"))
            cardurl = "https://img.scryfall.com/cards/normal/front/8/4/84dc847c-7a37-4c7f-b02c-30b3e4c91fb6.jpg?1561757490";
        else if (id.equals("8759611"))
            cardurl = "https://img.scryfall.com/cards/large/front/4/1/41004bdf-8e09-4b2c-9e9c-26c25eac9854.jpg?1562493483";
        else if (id.equals("8759911"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/b/0b61d772-2d8b-4acf-9dd2-b2e8b03538c8.jpg?1562492461";
        else if (id.equals("8759511"))
            cardurl = "https://img.scryfall.com/cards/large/front/d/2/d224c50f-8146-4c91-9401-04e5bd306d02.jpg?1562496100";
        else if (id.equals("8471611"))
            cardurl = "https://img.scryfall.com/cards/png/front/8/4/84920a21-ee2a-41ac-a369-347633d10371.png?1562494702";
        else if (id.equals("8760011"))
            cardurl = "https://img.scryfall.com/cards/large/front/4/2/42ba0e13-d20f-47f9-9c86-2b0b13c39ada.jpg?1562493487";
        else if (id.equals("401721"))
            cardurl = "https://deckmaster.info/images/cards/DDP/401721-hr.jpg";
        else if (id.equals("401722"))
            cardurl = "https://deckmaster.info/images/cards/DDP/401722-hr.jpg";
        else if (id.equals("19784311"))
            cardurl = "https://deckmaster.info/images/cards/AKH/-4173-hr.jpg";
        else if (id.equals("19784312"))
            cardurl = "https://deckmaster.info/images/cards/BNG/-10-hr.jpg";
        else if (id.equals("19784313"))
            cardurl = "https://deckmaster.info/images/cards/DDD/201843-hr.jpg";
        else if (id.equals("20787512"))
            cardurl = "https://deckmaster.info/images/cards/SOM/-227-hr.jpg";
        else if (id.equals("20787511"))
            cardurl = "https://deckmaster.info/images/cards/SOM/-226-hr.jpg";
        else if (id.equals("11492111"))
            cardurl = "https://deckmaster.info/images/cards/TSP/-2841-hr.jpg";
        else if (id.equals("11492112"))
            cardurl = "https://deckmaster.info/images/cards/TSP/-2840-hr.jpg";
        else if (id.equals("11492113"))
            cardurl = "https://img.scryfall.com/cards/large/front/5/b/5b9f471a-1822-4981-95a9-8923d83ddcbf.jpg?1562702075";
        else if (id.equals("11492114"))
            cardurl = "https://deckmaster.info/images/cards/DDN/386322-hr.jpg";
        else if (id.equals("11492115") || id.equals("209162"))
            cardurl = "https://deckmaster.info/images/cards/DDE/209162-hr.jpg";
        else if (id.equals("7448911"))
            cardurl = "https://img.scryfall.com/cards/large/front/c/a/ca03131a-9bd4-4fba-b95c-90f1831e86e7.jpg?1562879774";
        else if (id.equals("7453611"))
            cardurl = "https://img.scryfall.com/cards/large/front/7/3/73636ca0-2309-4bb3-9300-8bd0c0bb5b31.jpg?1562877808";
        else if (id.equals("7447611"))
            cardurl = "https://img.scryfall.com/cards/large/front/2/8/28f72260-c8f9-4c44-92b5-23cef6690fdd.jpg?1562876119";
        else if (id.equals("7467111"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/f/1fe2b76f-ddb7-49d5-933b-ccb06be5d46f.jpg?1562875903";
        else if (id.equals("7409311"))
            cardurl = "https://img.scryfall.com/cards/large/front/7/5/758abd53-6ad2-406e-8615-8e48678405b4.jpg?1562877848";
        else if (id.equals("3896122"))
            cardurl = "https://img.scryfall.com/cards/large/front/5/9/59a00cac-53ae-46ad-8468-e6d1db40b266.jpg?1562542382";
        else if (id.equals("3896522"))
            cardurl = "https://deckmaster.info/images/cards/C14/-474-hr.jpg";
        else if (id.equals("3896521"))
            cardurl = "https://deckmaster.info/images/cards/C14/-472-hr.jpg";
        else if (id.equals("3896523"))
            cardurl = "https://img.scryfall.com/cards/large/front/d/0/d0cd85cc-ad22-446b-8378-5eb69fee1959.jpg?1562840712";
        else if (id.equals("687701"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2437-hr.jpg";
        else if (id.equals("687702"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3069-hr.jpg";
        else if (id.equals("687703"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2443-hr.jpg";
        else if (id.equals("687704"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2444-hr.jpg";
        else if (id.equals("687705"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2450-hr.jpg";
        else if (id.equals("687713"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3175-hr.jpg";
        else if (id.equals("687712"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2624-hr.jpg";
        else if (id.equals("687711"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3168-hr.jpg";
        else if (id.equals("687710"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3161-hr.jpg";
        else if (id.equals("687709"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2485-hr.jpg";
        else if (id.equals("687752"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3085-hr.jpg";
        else if (id.equals("687707"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2478-hr.jpg";
        else if (id.equals("687751"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3083-hr.jpg";
        else if (id.equals("687720"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2652-hr.jpg";
        else if (id.equals("687719"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2650-hr.jpg";
        else if (id.equals("687718"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3178-hr.jpg";
        else if (id.equals("687717"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2641-hr.jpg";
        else if (id.equals("687716"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2634-hr.jpg";
        else if (id.equals("687715"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2631-hr.jpg";
        else if (id.equals("687714"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2630-hr.jpg";
        else if (id.equals("687722"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2550-hr.jpg";
        else if (id.equals("687721"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3183-hr.jpg";
        else if (id.equals("687734"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2398-hr.jpg";
        else if (id.equals("687708"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3086-hr.jpg";
        else if (id.equals("687732"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3158-hr.jpg";
        else if (id.equals("687731"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3157-hr.jpg";
        else if (id.equals("687755"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3156-hr.jpg";
        else if (id.equals("687730"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2603-hr.jpg";
        else if (id.equals("687729"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2576-hr.jpg";
        else if (id.equals("687728"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2573-hr.jpg";
        else if (id.equals("687727"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2570-hr.jpg";
        else if (id.equals("687726"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2568-hr.jpg";
        else if (id.equals("687725"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2559-hr.jpg";
        else if (id.equals("687724"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3131-hr.jpg";
        else if (id.equals("687723"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3128-hr.jpg";
        else if (id.equals("687740"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2759-hr.jpg";
        else if (id.equals("687739"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2755-hr.jpg";
        else if (id.equals("687738"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2432-hr.jpg";
        else if (id.equals("687737"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3053-hr.jpg";
        else if (id.equals("687756"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3054-hr.jpg";
        else if (id.equals("687736"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2408-hr.jpg";
        else if (id.equals("687735"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2403-hr.jpg";
        else if (id.equals("687733"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2729-hr.jpg";
        else if (id.equals("687706"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3082-hr.jpg";
        else if (id.equals("687750"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2748-hr.jpg";
        else if (id.equals("687748"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2747-hr.jpg";
        else if (id.equals("687749"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2746-hr.jpg";
        else if (id.equals("687742"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2743-hr.jpg";
        else if (id.equals("687743"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2744-hr.jpg";
        else if (id.equals("687744"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2745-hr.jpg";
        else if (id.equals("687745"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2763-hr.jpg";
        else if (id.equals("687746"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2764-hr.jpg";
        else if (id.equals("687747"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2765-hr.jpg";
        else if (id.equals("687741"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2761-hr.jpg";
        else if (id.equals("687753"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3176-hr.jpg";
        else if (id.equals("687754"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3184-hr.jpg";
        else if (id.equals("7897511"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/4/a4f4aa3b-c64a-4430-b1a2-a7fca87d0a22.jpg?1562763433";
        else if (id.equals("7868811"))
            cardurl = "https://img.scryfall.com/cards/large/front/b/3/b3523b8e-065f-427c-8d5b-eb731ca91ede.jpg?1562763691";
        else if (id.equals("7868711"))
            cardurl = "https://img.scryfall.com/cards/large/front/5/8/58164521-aeec-43fc-9db9-d595432dea6f.jpg?1564694999";
        else if (id.equals("7868611"))
            cardurl = "https://img.scryfall.com/cards/large/front/3/3/33a8e5b9-6bfb-4ff2-a16d-3168a5412807.jpg?1562758927";
        else if (id.equals("7869111"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/d/9de1eebf-5725-438c-bcf0-f3a4d8a89fb0.jpg?1562762993";
        else if (id.equals("7860011"))
            cardurl = "https://img.scryfall.com/cards/large/front/8/6/864ad989-19a6-4930-8efc-bbc077a18c32.jpg?1562762069";
        else if (id.equals("7867911"))
            cardurl = "https://img.scryfall.com/cards/large/front/c/8/c8265c39-d287-4c5a-baba-f2f09dd80a1c.jpg?1562764226";
        else if (id.equals("7867811"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/0/a00a7180-49bd-4ead-852a-67b6b5e4b933.jpg?1564694995";
        else if (id.equals("7869511"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/2/f2ddf1a3-e6fa-4dd0-b80d-1a585b51b934.jpg?1562765664";
        else if (id.equals("7869411"))
            cardurl = "https://img.scryfall.com/cards/large/front/6/e/6ee6cd34-c117-4d7e-97d1-8f8464bfaac8.jpg?1562761096";
        else if (id.equals("207998"))
            cardurl = "https://deckmaster.info/images/cards/DDE/207998-hr.jpg";
        else if (id.equals("19784555"))
            cardurl = "https://deckmaster.info/images/cards/DGM/-39-hr.jpg";
        else if (id.equals("19784612"))
            cardurl = "https://deckmaster.info/images/cards/RTR/-60-hr.jpg";
        else if (id.equals("19784613"))
            cardurl = "https://deckmaster.info/images/cards/RTR/-62-hr.jpg";
        else if (id.equals("19784611"))
            cardurl = "https://deckmaster.info/images/cards/RTR/-55-hr.jpg";
        else if (id.equals("4977511"))
            cardurl = "https://deckmaster.info/images/cards/DST/-2819-hr.jpg";
        else if (id.equals("4977512"))
            cardurl = "https://deckmaster.info/images/cards/DST/-2818-hr.jpg";

        return cardurl;
    }

    public static String getSpecialTokenUrl(String id) {
        String tokenurl = "";

        if (id.equals("75291t"))
            tokenurl = "http://4.bp.blogspot.com/-y5Fanm3qvrU/Vmd4gGnl2DI/AAAAAAAAAWY/FCrS9FTgOJk/s1600/Tatsumasa%2BToken.jpg";
        else if (id.equals("435411t") || id.equals("435410t"))
            tokenurl = "https://deckmaster.info/images/cards/XLN/-5173-hr.jpg";
        else if (id.equals("202474t") || id.equals("1098t") || id.equals("2024t") || id.equals("3766t") || id.equals("11183t") || id.equals("902t"))
            tokenurl = "https://deckmaster.info/images/cards/AST/-884-hr.jpg";
        else if (id.equals("202590t") || id.equals("2073t") || id.equals("1027t"))
            tokenurl = "https://deckmaster.info/images/cards/AST/-892-hr.jpg";
        else if (id.equals("201124t") || id.equals("3118t"))
            tokenurl = "https://deckmaster.info/images/cards/AL/-2029-hr.jpg";
        else if (id.equals("184735t") || id.equals("376488t") || id.equals("3066t") || id.equals("121261t"))
            tokenurl = "https://i.pinimg.com/originals/a9/fb/37/a9fb37bdfa8f8013b7eb854d155838e2.jpg";
        else if (id.equals("184598t"))
            tokenurl = "https://deckmaster.info/images/cards/HM/-2070-hr.jpg";
        else if (id.equals("184589t"))
            tokenurl = "https://deckmaster.info/images/cards/M14/-28-hr.jpg";
        else if (id.equals("184730t"))
            tokenurl = "https://deckmaster.info/images/cards/AL/-2028-hr.jpg";
        else if (id.equals("1649t") || id.equals("201182t"))
            tokenurl = "https://deckmaster.info/images/cards/LE/-2046-hr.jpg";
        else if (id.equals("140233t") || id.equals("191239t") || id.equals("205957t"))
            tokenurl = "https://deckmaster.info/card.php?multiverseid=-234";
        else if (id.equals("1686t") || id.equals("2881t") || id.equals("201231t"))
            tokenurl = "https://deckmaster.info/images/cards/A25/-5648-hr.jpg";
        else if (id.equals("368951t") || id.equals("426025t"))
            tokenurl = "https://deckmaster.info/card.php?multiverseid=-39";
        else if (id.equals("46168t"))
            tokenurl = "https://deckmaster.info/images/cards/KLD/-3287-hr.jpg";
        else if (id.equals("49026t"))
            tokenurl = "https://www.mtg.onl/static/a9d81341e62e39e75075b573739f39d6/4d406/PROXY_Wirefly_2_2.jpg";
        else if (id.equals("6142t"))
            tokenurl = "https://deckmaster.info/images/cards/EX/-2035-hr.jpg";
        else if (id.equals("126166t"))
            tokenurl = "https://deckmaster.info/images/cards/C14/-487-hr.jpg";
        else if (id.equals("136155t"))
            tokenurl = "https://i.pinimg.com/564x/5d/68/d6/5d68d67bef76bf90588a4afdc39dc60e.jpg";
        else if (id.equals("107091t"))
            tokenurl = "https://media.mtgsalvation.com/attachments/13/534/635032476540667501.jpg";
        else if (id.equals("452760t"))
            tokenurl = "https://deckmaster.info/images/cards/M19/-6036.jpg";
        else if (id.equals("2959t"))
            tokenurl = "https://deckmaster.info/images/cards/HM/-2070-hr.jpg";
        else if (id.equals("380486t"))
            tokenurl = "https://deckmaster.info/images/cards/BNG/-5-hr.jpg";
        else if (id.equals("380487t") || id.equals("414506t"))
            tokenurl = "https://deckmaster.info/images/cards/JOU/-41-hr.jpg";
        else if (id.equals("234849t"))
            tokenurl = "https://deckmaster.info/images/cards/RTR/-61-hr.jpg";
        else if (id.equals("23319t"))
            tokenurl = "https://www.mtg.onl/static/0f8b0552293c03a3a29614cc83024337/4d406/PROXY_Reflection_W_X_X.jpg";
        else if (id.equals("205297t") || id.equals("50104t"))
            tokenurl = "https://i.pinimg.com/564x/cc/96/e3/cc96e3bdbe7e0f4bf1c0c1f942c073a9.jpg";
        else if (id.equals("3449t"))
            tokenurl = "https://www.mtg.onl/static/8c7fed1a0b8edd97c0fb0ceab24a654f/4d406/PROXY_Goblin_Scout_R_1_1.jpg";
        else if (id.equals("3392t"))
            tokenurl = "https://deckmaster.info/images/cards/DDR/417498-hr.jpg";
        else if (id.equals("3280t"))
            tokenurl = "https://media.mtgsalvation.com/attachments/54/421/635032484680831888.jpg";
        else if (id.equals("3242t"))
            tokenurl = "https://deckmaster.info/images/cards/MI/-2828-hr.jpg";
        else if (id.equals("19878t"))
            tokenurl = "https://deckmaster.info/images/cards/C14/-482-hr.jpg";
        else if (id.equals("21381t") || id.equals("40198t"))
            tokenurl = "https://img.scryfall.com/cards/large/back/8/c/8ce60642-e207-46e6-b198-d803ff3b47f4.jpg?1562921132";
        else if (id.equals("265141t"))
            tokenurl = "https://deckmaster.info/images/cards/VMA/-4465-hr.jpg";
        else if (id.equals("24624t"))
            tokenurl = "https://www.mtg.onl/static/6d717cba653ea9e3f6bd1419741671cb/4d406/PROXY_Minion_B_1_1.jpg";
        else if (id.equals("409810t") || id.equals("409805t") || id.equals("409953t") || id.equals("409997t") || id.equals("410032t"))
            tokenurl = "https://deckmaster.info/images/cards/SOI/-2404-hr.jpg";
        else if (id.equals("74492t"))
            tokenurl = "https://media.mtgsalvation.com/attachments/94/295/635032496473215708.jpg";
        else if (id.equals("88973t"))
            tokenurl = "https://deckmaster.info/images/cards/DDQ/409655-hr.jpg";
        else if (id.equals("89051t"))
            tokenurl = "https://www.mtg.onl/static/b7625a256e10bcec251a1a0abbf17bd4/4d406/PROXY_Horror_B_4_4.jpg";
        else if (id.equals("5261t"))
            tokenurl = "https://static.cardmarket.com/img/5a0199344cad68eebeefca6fa24e52c3/items/1/MH1/376905.jpg";
        else if (id.equals("116384t") || id.equals("376564t"))
            tokenurl = "https://deckmaster.info/images/cards/TSP/-114916-hr.jpg";
        else if (id.equals("116383t"))
            tokenurl = "https://deckmaster.info/images/cards/TSP/-2170-hr.jpg";
        else if (id.equals("114917t"))
            tokenurl = "https://deckmaster.info/images/cards/JOU/-43-hr.jpg";
        else if (id.equals("5610t"))
            tokenurl = "https://deckmaster.info/images/cards/DDE/207998-hr.jpg";
        else if (id.equals("185704t"))
            tokenurl = "https://deckmaster.info/images/cards/ZEN/-277-hr.jpg";
        else if (id.equals("461099t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/d/e/de7ba875-f77b-404f-8b75-4ba6f81da410.jpg?1557575978";
        else if (id.equals("9667t"))
            tokenurl = "https://deckmaster.info/images/cards/UG/-2062-hr.jpg";
        else if (id.equals("368549t"))
            tokenurl = "https://deckmaster.info/images/cards/DDQ/409655-hr.jpg";
        else if (id.equals("73953t"))
            tokenurl = "https://deckmaster.info/images/cards/UNH/-2065-hr.jpg";
        else if (id.equals("74265t"))
            tokenurl = "https://deckmaster.info/images/cards/UNH/-2064-hr.jpg";
        else if (id.equals("27634t"))
            tokenurl = "https://deckmaster.info/images/cards/PS/-2072-hr.jpg";
        else if (id.equals("111046t"))
            tokenurl = "https://deckmaster.info/images/cards/PLC/-2071-hr.jpg";
        else if (id.equals("4771t"))
            tokenurl = "https://deckmaster.info/images/cards/TE/-2060-hr.jpg";
        else if (id.equals("3591t"))
            tokenurl = "https://i.pinimg.com/564x/6e/8d/fe/6e8dfeee2919a3efff210df56ab7b85d.jpg";
        else if (id.equals("72858t"))
            tokenurl = "https://www.mtg.onl/static/348314ede9097dd8f6dd018a6502d125/4d406/PROXY_Pincher_2_2.jpg";
        else if (id.equals("3832t"))
            tokenurl = "https://deckmaster.info/images/cards/GK1_DIMIR/-6541-hr.jpg";
        else if (id.equals("426909t") || id.equals("426705t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/9/8/98956e73-04e4-4d7f-bda5-cfa78eb71350.jpg?1562844807";
        else if (id.equals("426897t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/a/8/a8f339c6-2c0d-4631-849b-44d4360b5131.jpg?1562844814";
        else if (id.equals("175105t"))
            tokenurl = "https://deckmaster.info/images/cards/ALA/-325-hr.jpg";
        else if (id.equals("470549t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/7/7/7711a586-37f9-4560-b25d-4fb339d9cd55.jpg?1565299650";
        else if (id.equals("3227t"))
            tokenurl = "https://deckmaster.info/images/cards/PS/-2072-hr.jpg";
        else if (id.equals("3148t"))
            tokenurl = "https://deckmaster.info/images/cards/AL/-2156-hr.jpg";
        else if (id.equals("3113t"))
            tokenurl = "https://www.mtg.onl/static/fca7508d78c26e3daea78fd4640faf9a/4d406/PROXY_Orb_U_X_X.jpg";
        else if (id.equals("26815t"))
            tokenurl = "https://deckmaster.info/images/cards/AP/-2163-hr.jpg";
        else if (id.equals("25956t"))
            tokenurl = "https://deckmaster.info/images/cards/AP/-2069-hr.jpg";
        else if (id.equals("74027t"))
            tokenurl = "https://www.mtg.onl/static/48515f01d0fda15dd9308d3a528dae7b/4d406/PROXY_Spirit_W_3_3.jpg";

        return tokenurl;
    }

    public static boolean hasToken(String id) {
        if (id.equals("456378") || id.equals("2912") || id.equals("1514") || id.equals("364") || id.equals("69") || id.equals("369012") ||
                id.equals("417759") || id.equals("386476") || id.equals("456371") || id.equals("456360") || id.equals("391958") || id.equals("466959") ||
                id.equals("466813") || id.equals("201176") || id.equals("202483") || id.equals("3546") || id.equals("425949") || id.equals("426027") ||
                id.equals("425853") || id.equals("425846") || id.equals("426036") || id.equals("370387") || id.equals("29955") || id.equals("29989") ||
                id.equals("19741") || id.equals("19722") || id.equals("19706") || id.equals("24597") || id.equals("24617") || id.equals("24563") ||
                id.equals("253539") || id.equals("277995") || id.equals("265415") || id.equals("289225") || id.equals("289215") || id.equals("253529") ||
                id.equals("253641") || id.equals("270957") || id.equals("401685") || id.equals("89116") || id.equals("5183") || id.equals("5177") ||
                id.equals("209289") || id.equals("198171") || id.equals("10419") || id.equals("470542") || id.equals("29992") || id.equals("666") ||
                id.equals("2026") || id.equals("45395") || id.equals("442021") || id.equals("423758") || id.equals("426930") || id.equals("998") ||
                id.equals("446163") || id.equals("378411") || id.equals("376457") || id.equals("470749") || id.equals("450641") || id.equals("470623") ||
                id.equals("470620") || id.equals("470754") || id.equals("470750") || id.equals("470739") || id.equals("470708") || id.equals("470581") ||
                id.equals("470578") || id.equals("470571") || id.equals("470552") || id.equals("394490") || id.equals("114921"))
            return false;
        return true;
    }

    public static Document findTokenPage(String imageurl, String name, String set, String[] availableSets, String tokenstats, SDLActivity parent) throws Exception {
        Document doc = null;
        Elements outlinks = null;
        try {
            doc = Jsoup.connect(imageurl + "t" + set.toLowerCase()).get();
            if (doc != null) {
                outlinks = doc.select("body a");
                if (outlinks != null) {
                    for (int k = 0; k < outlinks.size() && parent.downloadInProgress; k++) {
                        while (parent.paused && parent.downloadInProgress) {
                            try {
                                Thread.sleep(1000);
                            } catch (InterruptedException e) {
                            }
                        }
                        if (!parent.downloadInProgress)
                            break;
                        String linktoken = outlinks.get(k).attributes().get("href");
                        if (linktoken != null && !linktoken.isEmpty()) {
                            try {
                                Document tokendoc = Jsoup.connect(linktoken).get();
                                if (tokendoc == null)
                                    continue;
                                Elements stats = tokendoc.select("head meta");
                                if (stats != null) {
                                    for (int j = 0; j < stats.size() && parent.downloadInProgress; j++) {
                                        while (parent.paused && parent.downloadInProgress) {
                                            try {
                                                Thread.sleep(1000);
                                            } catch (InterruptedException e) {
                                            }
                                        }
                                        if (!parent.downloadInProgress)
                                            break;
                                        String a = stats.get(j).attributes().get("content");
                                        if (stats.get(j).attributes().get("content").contains(tokenstats) &&
                                                stats.get(j).attributes().get("content").toLowerCase().contains(name.toLowerCase())) {
                                            return tokendoc;
                                        }
                                    }
                                }
                            } catch (Exception e) {
                            }
                        }
                    }
                }
            }
        } catch (Exception e) {
        }
        System.out.println("Warning: Token " + name + " has not been found between " + set + " tokens, i will search for it in https://deckmaster.info");
        String json = "";
        try {
            URL url = new URL("https://deckmaster.info/includes/ajax.php?action=cardSearch&searchString=" + name);
            HttpURLConnection httpcon = (HttpURLConnection) url.openConnection();
            if (httpcon != null) {
                httpcon.addRequestProperty("User-Agent", "Mozilla/4.76");
                InputStream stream = httpcon.getInputStream();
                if (stream != null) {
                    int i;
                    while ((i = stream.read()) != -1) {
                        json = json + ((char) i);
                    }
                }
            }
        } catch (Exception e) {
        }
        List<String> urls = new ArrayList<String>();
        String[] tok = json.split(",");
        for (int i = 0; i < tok.length; i++) {
            if (tok[i].contains("multiverseid")) {
                String id = tok[i].split(":")[1].replace("\"", "");
                urls.add(id);
            }
        }
        for (int i = 0; i < urls.size() && parent.downloadInProgress; i++) {
            while (parent.paused && parent.downloadInProgress) {
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                }
            }
            if (!parent.downloadInProgress)
                break;
            try {
                Document tokendoc = Jsoup.connect("https://deckmaster.info/card.php?multiverseid=" + urls.get(i)).get();
                if (tokendoc == null)
                    continue;
                Elements stats = tokendoc.select("head meta");
                if (stats != null) {
                    for (int j = 0; j < stats.size() && parent.downloadInProgress; j++) {
                        while (parent.paused && parent.downloadInProgress) {
                            try {
                                Thread.sleep(1000);
                            } catch (InterruptedException e) {
                            }
                        }
                        if (!parent.downloadInProgress)
                            break;
                        if (stats.get(j).attributes().get("content").contains("Token Creature") && stats.get(j).attributes().get("content").toLowerCase().contains(name.toLowerCase())) {
                            if (stats.get(j).attributes().get("content").contains(tokenstats.replace("X/X", "★/★")))
                                return tokendoc;
                            stats = tokendoc.select("body textarea");
                            if (stats != null) {
                                for (int y = 0; y < stats.size() && parent.downloadInProgress; y++) {
                                    while (parent.paused && parent.downloadInProgress) {
                                        try {
                                            Thread.sleep(1000);
                                        } catch (InterruptedException e) {
                                        }
                                    }
                                    if (!parent.downloadInProgress)
                                        break;
                                    List<Node> nodes = stats.get(y).childNodes();
                                    if (nodes != null) {
                                        for (int p = 0; p < nodes.size() && parent.downloadInProgress; p++) {
                                            while (parent.paused && parent.downloadInProgress) {
                                                try {
                                                    Thread.sleep(1000);
                                                } catch (InterruptedException e) {
                                                }
                                            }
                                            if (!parent.downloadInProgress)
                                                break;
                                            if (stats.get(y).childNode(p).attributes().get("#text").contains(tokenstats))
                                                return tokendoc;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            } catch (Exception e) {
            }
        }
        System.out.println("Warning: Token " + name + " has not been found with an indexed search algorithm in https://deckmaster.info so i will try with a deeper searching algorithm (it may take a long time)");
        for (int i = 1; i < 6563 && parent.downloadInProgress; i++) {
            if (Arrays.binarySearch(tokenids, i) < 0)
                continue;
            while (parent.paused && parent.downloadInProgress) {
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                }
            }
            if (!parent.downloadInProgress)
                break;
            try {
                Document tokendoc = Jsoup.connect("https://deckmaster.info/card.php?multiverseid=-" + i).get();
                if (tokendoc == null)
                    continue;
                Elements stats = tokendoc.select("head meta");
                if (stats != null) {
                    for (int j = 0; j < stats.size() && parent.downloadInProgress; j++) {
                        while (parent.paused && parent.downloadInProgress) {
                            try {
                                Thread.sleep(1000);
                            } catch (InterruptedException e) {
                            }
                        }
                        if (!parent.downloadInProgress)
                            break;
                        if (stats.get(j).attributes().get("content").contains("Token Creature") &&
                                stats.get(j).attributes().get("content").toLowerCase().contains(name.toLowerCase())) {
                            if (stats.get(j).attributes().get("content").contains(tokenstats.replace("X/X", "★/★")))
                                return tokendoc;
                            stats = tokendoc.select("body textarea");
                            if (stats != null) {
                                for (int y = 0; y < stats.size() && parent.downloadInProgress; y++) {
                                    while (parent.paused && parent.downloadInProgress) {
                                        try {
                                            Thread.sleep(1000);
                                        } catch (InterruptedException e) {
                                        }
                                    }
                                    if (!parent.downloadInProgress)
                                        break;
                                    List<Node> nodes = stats.get(y).childNodes();
                                    if (nodes != null) {
                                        for (int p = 0; p < nodes.size() && parent.downloadInProgress; p++) {
                                            while (parent.paused && parent.downloadInProgress) {
                                                try {
                                                    Thread.sleep(1000);
                                                } catch (InterruptedException e) {
                                                }
                                            }
                                            if (!parent.downloadInProgress)
                                                break;
                                            if (stats.get(y).childNode(p).attributes().get("#text").contains(tokenstats))
                                                return tokendoc;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            } catch (Exception e) {
            }
        }
        System.out.println("Warning: Token " + name + " has not been found in https://deckmaster.info so i will search for it between any other set in " + imageurl + " (it may take a long time)");
        for (int i = 0; i < availableSets.length; i++) {
            String currentSet = availableSets[i].toLowerCase().split(" - ")[0];
            if (!currentSet.equalsIgnoreCase(set)) {
                try {
                    doc = Jsoup.connect(imageurl + "t" + currentSet).get();
                    if (doc == null)
                        continue;
                    outlinks = doc.select("body a");
                    if (outlinks != null) {
                        for (int k = 0; k < outlinks.size() && parent.downloadInProgress; k++) {
                            while (parent.paused && parent.downloadInProgress) {
                                try {
                                    Thread.sleep(1000);
                                } catch (InterruptedException e) {
                                }
                            }
                            if (!parent.downloadInProgress)
                                break;
                            String linktoken = outlinks.get(k).attributes().get("href");
                            try {
                                Document tokendoc = Jsoup.connect(linktoken).get();
                                if (tokendoc == null)
                                    continue;
                                Elements stats = tokendoc.select("head meta");
                                if (stats != null) {
                                    for (int j = 0; j < stats.size() && parent.downloadInProgress; j++) {
                                        while (parent.paused && parent.downloadInProgress) {
                                            try {
                                                Thread.sleep(1000);
                                            } catch (InterruptedException e) {
                                            }
                                        }
                                        if (!parent.downloadInProgress)
                                            break;
                                        String a = stats.get(j).attributes().get("content");
                                        if (stats.get(j).attributes().get("content").contains(tokenstats) && stats.get(j).attributes().get("content").toLowerCase().contains(name.toLowerCase())) {
                                            System.out.println("Token " + name + " has been found between " + currentSet.toUpperCase() + " tokens, i will use this one");
                                            return tokendoc;
                                        }
                                    }
                                }
                            } catch (Exception e) {
                            }
                        }
                    }
                } catch (Exception e) {
                }
            }
        }
        System.err.println("Error: Token " + name + " has not been found between any set of " + imageurl);
        throw new Exception();
    }

    public static String DownloadCardImages(String set, String[] availableSets, String targetres, String basePath, String destinationPath, ProgressDialog progressBarDialog, SDLActivity parent) throws IOException {
        String res = "";

        String baseurl = "https://gatherer.wizards.com/Pages/Card/Details.aspx?multiverseid=";
        String imageurl = "https://scryfall.com/sets/";

        Integer ImgX = 0;
        Integer ImgY = 0;
        Integer ThumbX = 0;
        Integer ThumbY = 0;

        if (targetres.equals("High")) {
            ImgX = 672;
            ImgY = 936;
            ThumbX = 124;
            ThumbY = 176;
        } else if (targetres.equals("Medium")) {
            ImgX = 488;
            ImgY = 680;
            ThumbX = 90;
            ThumbY = 128;
        } else if (targetres.equals("Low")) {
            ImgX = 244;
            ImgY = 340;
            ThumbX = 45;
            ThumbY = 64;
        } else if (targetres.equals("Tiny")) {
            ImgX = 180;
            ImgY = 255;
            ThumbX = 45;
            ThumbY = 64;
        }

        File baseFolder = new File(basePath);
        File[] listOfFiles = baseFolder.listFiles();
        Map<String, String> mappa = new HashMap<String, String>();
        ZipFile zipFile = null;
        InputStream stream = null;
        java.nio.file.Path filePath = null;
        try {
            zipFile = new ZipFile(basePath + File.separator + listOfFiles[0].getName());
            Enumeration<? extends ZipEntry> e = zipFile.entries();
            while (e.hasMoreElements()) {
                ZipEntry entry = e.nextElement();
                String entryName = entry.getName();
                if (entryName != null && entryName.contains("sets" + File.separator)) {
                    if (entryName.contains("_cards.dat")) {
                        String[] names = entryName.split(File.separator);
                        if (set.equalsIgnoreCase(names[1])) {
                            stream = zipFile.getInputStream(entry);
                            byte[] buffer = new byte[1];
                            java.nio.file.Path outDir = Paths.get(basePath);
                            filePath = outDir.resolve("_cards.dat");
                            try {
                                FileOutputStream fos = new FileOutputStream(filePath.toFile());
                                BufferedOutputStream bos = new BufferedOutputStream(fos, buffer.length);
                                int len;
                                while ((len = stream.read(buffer)) != -1) {
                                    bos.write(buffer, 0, len);
                                }
                                fos.close();
                                bos.close();
                            } catch (Exception ex) {
                                System.out.println("Error extracting zip file" + ex);
                            }
                            break;
                        }
                    }
                }
            }
        } catch (IOException ioe) {
            System.out.println("Error opening zip file" + ioe);
        } finally {
            try {
                if (zipFile != null) {
                    zipFile.close();
                }
            } catch (IOException ioe) {
                System.out.println("Error while closing zip file" + ioe);
            }
        }

        String lines = readLineByLineJava8(filePath.toString());
        File del = new File(filePath.toString());
        del.delete();
        int totalcards = 0;
        String findStr = "total=";
        int lastIndex = lines.indexOf(findStr);
        String totals = lines.substring(lastIndex, lines.indexOf("\n", lastIndex));
        totalcards = Integer.parseInt(totals.split("=")[1]);
        while (lines.contains("[card]")) {
            findStr = "[card]";
            lastIndex = lines.indexOf(findStr);
            String id = null;
            String primitive = null;
            int a = lines.indexOf("primitive=", lastIndex);
            if (a > 0)
                primitive = lines.substring(a, lines.indexOf("\n", a)).replace("//", "-").split("=")[1];
            int b = lines.indexOf("id=", lastIndex);
            if (b > 0)
                id = lines.substring(b, lines.indexOf("\n", b)).replace("-", "").split("=")[1];
            int c = lines.indexOf("[/card]", lastIndex);
            if (c > 0)
                lines = lines.substring(c + 8);
            if (primitive != null && id != null && !id.equalsIgnoreCase("null"))
                mappa.put(id, primitive);
            if (id.equals("114921")) {
                mappa.put("11492111", "Citizen");
                mappa.put("11492112", "Camarid");
                mappa.put("11492113", "Thrull");
                mappa.put("11492114", "Goblin");
                mappa.put("11492115", "Saproling");
            }
        }

        progressBarDialog.setProgress(0);
        progressBarDialog.setMax(totalcards);

        File imgPath = new File(destinationPath + set + File.separator);
        if (!imgPath.exists()) {
            System.out.println("creating directory: " + imgPath.getName());
            boolean result = false;
            try {
                imgPath.mkdir();
                result = true;
            } catch (SecurityException se) {
                System.err.println(imgPath + " not created");
                System.exit(1);
            }
            if (result) {
                System.out.println(imgPath + " created");
            }
        }

        File thumbPath = new File(destinationPath + set + File.separator + "thumbnails" + File.separator);
        if (!thumbPath.exists()) {
            System.out.println("creating directory: " + thumbPath.getName());
            boolean result = false;
            try {
                thumbPath.mkdir();
                result = true;
            } catch (SecurityException se) {
                System.err.println(thumbPath + " not created");
                System.exit(1);
            }
            if (result) {
                System.out.println(thumbPath + " created");
            }
        }

        String scryset = set;
        if (scryset.equalsIgnoreCase("MRQ"))
            scryset = "MMQ";
        else if (scryset.equalsIgnoreCase("AVN"))
            scryset = "DDH";
        else if (scryset.equalsIgnoreCase("BVC"))
            scryset = "DDQ";
        else if (scryset.equalsIgnoreCase("CFX"))
            scryset = "CON";
        else if (scryset.equalsIgnoreCase("DM"))
            scryset = "DKM";
        else if (scryset.equalsIgnoreCase("EVK"))
            scryset = "DDO";
        else if (scryset.equalsIgnoreCase("EVT"))
            scryset = "DDF";
        else if (scryset.equalsIgnoreCase("FVD"))
            scryset = "DRB";
        else if (scryset.equalsIgnoreCase("FVE"))
            scryset = "V09";
        else if (scryset.equalsIgnoreCase("FVL"))
            scryset = "V11";
        else if (scryset.equalsIgnoreCase("FVR"))
            scryset = "V10";
        else if (scryset.equalsIgnoreCase("HVM"))
            scryset = "DDL";
        else if (scryset.equalsIgnoreCase("IVG"))
            scryset = "DDJ";
        else if (scryset.equalsIgnoreCase("JVV"))
            scryset = "DDM";
        else if (scryset.equalsIgnoreCase("KVD"))
            scryset = "DDG";
        else if (scryset.equalsIgnoreCase("PDS"))
            scryset = "H09";
        else if (scryset.equalsIgnoreCase("PVC"))
            scryset = "DDE";
        else if (scryset.equalsIgnoreCase("RV"))
            scryset = "3ED";
        else if (scryset.equalsIgnoreCase("SVT"))
            scryset = "DDK";
        else if (scryset.equalsIgnoreCase("VVK"))
            scryset = "DDI";
        else if (scryset.equalsIgnoreCase("ZVE"))
            scryset = "DDP";

        for (int y = 0; y < mappa.size() && parent.downloadInProgress; y++) {
            while (parent.paused && parent.downloadInProgress) {
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                }
            }
            if (!parent.downloadInProgress)
                break;

            String id = mappa.keySet().toArray()[y].toString();
            progressBarDialog.incrementProgressBy((int) (1));
            String specialcardurl = getSpecialCardUrl(id);
            if (!specialcardurl.isEmpty()) {
                URL url = new URL(specialcardurl);
                HttpURLConnection httpcon = (HttpURLConnection) url.openConnection();
                if (httpcon == null) {
                    System.err.println("Error: Problem fetching card: " + mappa.get(id) + "-" + id + ", i will not download it...");
                    break;
                }
                httpcon.addRequestProperty("User-Agent", "Mozilla/4.76");
                InputStream in = null;
                try {
                    in = new BufferedInputStream(httpcon.getInputStream());
                } catch (Exception ex) {
                    System.out.println("Warning: Problem downloading card: " + mappa.get(id) + "-" + id + " from " + scryset + " on ScryFall, i will retry 2 times more...");
                    try {
                        in = new BufferedInputStream(url.openStream());
                    } catch (Exception ex2) {
                        System.out.println("Warning: Problem downloading card: " + mappa.get(id) + "-" + id + " from " + scryset + " on ScryFall, i will retry 1 time more...");
                        try {
                            in = new BufferedInputStream(url.openStream());
                        } catch (Exception ex3) {
                            System.err.println("Error: Problem downloading card: " + mappa.get(id) + "-" + id + " from " + scryset + " on ScryFall, i will not retry anymore...");
                            break;
                        }
                    }
                }
                ByteArrayOutputStream out = new ByteArrayOutputStream();
                byte[] buf = new byte[1024];
                int n = 0;
                while (-1 != (n = in.read(buf))) {
                    out.write(buf, 0, n);
                }
                out.close();
                in.close();
                byte[] response = out.toByteArray();
                String cardimage = imgPath + File.separator + id + ".jpg";
                String thumbcardimage = thumbPath + File.separator + id + ".jpg";
                if (id.equals("11492111") || id.equals("11492112") || id.equals("11492113") ||
                        id.equals("11492114") || id.equals("11492115")) {
                    cardimage = imgPath + File.separator + id + "t.jpg";
                    thumbcardimage = thumbPath + File.separator + id + "t.jpg";
                }
                FileOutputStream fos = new FileOutputStream(cardimage);
                fos.write(response);
                fos.close();

                Bitmap yourBitmap = BitmapFactory.decodeFile(cardimage);
                Bitmap resized = Bitmap.createScaledBitmap(yourBitmap, ImgX, ImgY, true);
                try {
                    FileOutputStream fout = new FileOutputStream(cardimage);
                    resized.compress(Bitmap.CompressFormat.JPEG, 100, fout);
                } catch (IOException e) {
                    e.printStackTrace();
                }
                Bitmap resizedThumb = Bitmap.createScaledBitmap(yourBitmap, ThumbX, ThumbY, true);
                try {
                    FileOutputStream fout = new FileOutputStream(thumbcardimage);
                    resizedThumb.compress(Bitmap.CompressFormat.JPEG, 100, fout);
                } catch (IOException e) {
                    e.printStackTrace();
                }
                continue;
            }
            Document doc = null;
            try {
                doc = Jsoup.connect(baseurl + id).get();
            } catch (Exception e) {
                System.out.println("Warning: Problem reading card (" + mappa.get(id) + ") infos from: " + baseurl + id + ", i will retry 2 times more...");
                try {
                    doc = Jsoup.connect(baseurl + id).get();
                } catch (Exception e2) {
                    System.out.println("Warning: Problem reading card (" + mappa.get(id) + ") infos from: " + baseurl + id + ", i will retry 1 time more...");
                    try {
                        doc = Jsoup.connect(baseurl + id).get();
                    } catch (Exception e3) {
                        System.err.println("Error: Problem reading card (" + mappa.get(id) + ") infos from: " + baseurl + id + ", i will not retry anymore...");
                        continue;
                    }
                }
            }
            if (doc == null) {
                System.err.println("Error: Problem reading card (" + mappa.get(id) + ") infos from: " + baseurl + id + ", i can't download it...");
                continue;
            }
            Elements divs = doc.select("body div");
            if (divs == null) {
                System.err.println("Error: Problem reading card (" + mappa.get(id) + ") infos from: " + baseurl + id + ", i can't download it...");
                continue;
            }

            int k;
            for (k = 0; k < divs.size(); k++)
                if (divs.get(k).childNodes().size() > 0 && divs.get(k).childNode(0).toString().toLowerCase().contains("card name"))
                    break;
            if (k >= divs.size()) {
                System.err.println("Error: Problem reading card (" + mappa.get(id) + ") infos from: " + baseurl + id + ", i can't download it...");
                continue;
            }
            String cardname = divs.get(k + 1).childNode(0).attributes().get("#text").replace("\r\n", "").trim();

            while (parent.paused && parent.downloadInProgress) {
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                }
            }
            if (!parent.downloadInProgress)
                break;

            if (scryset.equals("UST") || scryset.equals("S00")) {
                cardname = cardname.replace(" (a)", "");
                cardname = cardname.replace(" (b)", "");
                cardname = cardname.replace(" (c)", "");
                cardname = cardname.replace(" (d)", "");
                cardname = cardname.replace(" (e)", "");
                cardname = cardname.replace(" (f)", "");
                cardname = cardname.replace(" ...", "");
                String deckutrl = "https://deckmaster.info/card.php?multiverseid=";
                try {
                    doc = Jsoup.connect(deckutrl + id).get();
                } catch (Exception e) {
                    System.out.println("Warning: Problem reading card (" + mappa.get(id) + ") infos from: " + deckutrl + id + ", i will retry 2 times more...");
                    try {
                        doc = Jsoup.connect(deckutrl + id).get();
                    } catch (Exception e2) {
                        System.out.println("Warning: Problem reading card (" + mappa.get(id) + ") infos from: " + deckutrl + id + ", i will retry 1 time more...");
                        try {
                            doc = Jsoup.connect(deckutrl + id).get();
                        } catch (Exception e3) {
                            System.err.println("Error: Problem reading card (" + mappa.get(id) + ") infos from: " + deckutrl + id + ", i will not retry anymore...");
                            continue;
                        }
                    }
                }
            } else if (targetres.equals("High")) {
                try {
                    doc = Jsoup.connect(imageurl + scryset.toLowerCase()).get();
                    Elements outlinks = doc.select("body a");
                    if (outlinks != null) {
                        for (int h = 0; h < outlinks.size(); h++) {
                            String linkcard = outlinks.get(h).attributes().get("href");
                            if (linkcard != null && linkcard.contains(cardname.toLowerCase().replace(" ", "-"))) {
                                try {
                                    doc = Jsoup.connect(linkcard).get();
                                    if (doc == null)
                                        continue;
                                    Elements metadata = doc.select("head meta");
                                    if (metadata != null) {
                                        for (int j = 0; j < metadata.size(); j++) {
                                            if (metadata.get(j).attributes().get("content").toLowerCase().contains(cardname.toLowerCase())) {
                                                h = outlinks.size();
                                                break;
                                            }
                                        }
                                    }
                                } catch (Exception ex) {
                                }
                            }
                        }
                    }
                } catch (Exception e) {
                    System.out.println("Warning: Problem downloading card: " + mappa.get(id) + "-" + id + " from " + scryset + " on ScryFall, i will retry 2 times more...");
                    try {
                        doc = Jsoup.connect(imageurl + scryset.toLowerCase()).get();
                        Elements outlinks = doc.select("body a");
                        if (outlinks != null) {
                            for (int h = 0; h < outlinks.size(); h++) {
                                String linkcard = outlinks.get(h).attributes().get("href");
                                if (linkcard != null && linkcard.contains(cardname.toLowerCase().replace(" ", "-"))) {
                                    try {
                                        doc = Jsoup.connect(linkcard).get();
                                        if (doc == null)
                                            continue;
                                        Elements metadata = doc.select("head meta");
                                        if (metadata != null) {
                                            for (int j = 0; j < metadata.size(); j++) {
                                                if (metadata.get(j).attributes().get("content").toLowerCase().contains(cardname.toLowerCase())) {
                                                    h = outlinks.size();
                                                    break;
                                                }
                                            }
                                        }
                                    } catch (Exception ex) {
                                    }
                                }
                            }
                        }
                    } catch (Exception e2) {
                        System.out.println("Warning: Problem downloading card: " + mappa.get(id) + "-" + id + " from " + scryset + " on ScryFall, i will retry 1 time more...");
                        try {
                            doc = Jsoup.connect(imageurl + scryset.toLowerCase()).get();
                            Elements outlinks = doc.select("body a");
                            if (outlinks != null) {
                                for (int h = 0; h < outlinks.size(); h++) {
                                    String linkcard = outlinks.get(h).attributes().get("href");
                                    if (linkcard != null && linkcard.contains(cardname.toLowerCase().replace(" ", "-"))) {
                                        try {
                                            doc = Jsoup.connect(linkcard).get();
                                            if (doc == null)
                                                continue;
                                            Elements metadata = doc.select("head meta");
                                            if (metadata != null) {
                                                for (int j = 0; j < metadata.size(); j++) {
                                                    if (metadata.get(j).attributes().get("content").toLowerCase().contains(cardname.toLowerCase())) {
                                                        h = outlinks.size();
                                                        break;
                                                    }
                                                }
                                            }
                                        } catch (Exception ex) {
                                        }
                                    }
                                }
                            }
                        } catch (Exception e3) {
                            System.err.println("Error: Problem downloading card: " + mappa.get(id) + "-" + id + " from " + scryset + " on ScryFall, i will not retry anymore...");
                            continue;
                        }
                    }
                }
            } else {
                try {
                    doc = Jsoup.connect(imageurl + scryset.toLowerCase()).get();
                } catch (Exception e) {
                    System.out.println("Warning: Problem downloading card: " + mappa.get(id) + "-" + id + " from " + scryset + " on ScryFall, i will retry 2 times more...");
                    try {
                        doc = Jsoup.connect(imageurl + scryset.toLowerCase()).get();
                    } catch (Exception e2) {
                        System.out.println("Warning: Problem downloading card: " + mappa.get(id) + "-" + id + " from " + scryset + " on ScryFall, i will retry 1 time more...");
                        try {
                            doc = Jsoup.connect(imageurl + scryset.toLowerCase()).get();
                        } catch (Exception e3) {
                            System.err.println("Error: Problem downloading card: " + mappa.get(id) + "-" + id + " from " + scryset + " on ScryFall, i will not retry anymore...");
                            res = mappa.get(id) + " - " + set + File.separator + id + ".jpg\n" + res;
                            continue;
                        }
                    }
                }
            }

            if (doc == null) {
                System.err.println("Error: Problem fetching card: " + mappa.get(id) + "-" + id + " from " + scryset + " on ScryFall, i will not download it...");
                continue;
            }

            Elements imgs = doc.select("body img");
            if (imgs == null) {
                System.err.println("Error: Problem fetching card: " + mappa.get(id) + "-" + id + " from " + scryset + " on ScryFall, i will not download it...");
                continue;
            }

            for (int i = 0; i < imgs.size() && parent.downloadInProgress; i++) {
                while (parent.paused && parent.downloadInProgress) {
                    try {
                        Thread.sleep(1000);
                    } catch (InterruptedException e) {
                    }
                }
                if (!parent.downloadInProgress)
                    break;

                String title = imgs.get(i).attributes().get("alt");
                if (title.isEmpty())
                    title = imgs.get(i).attributes().get("title");
                if (title.toLowerCase().contains(cardname.toLowerCase())) {
                    String CardImage = imgs.get(i).attributes().get("src");
                    if (CardImage.isEmpty())
                        CardImage = imgs.get(i).attributes().get("data-src");
                    URL url = new URL(CardImage);
                    HttpURLConnection httpcon = (HttpURLConnection) url.openConnection();
                    if (httpcon == null) {
                        System.err.println("Error: Problem fetching card: " + mappa.get(id) + "-" + id + ", i will not download it...");
                        break;
                    }
                    httpcon.addRequestProperty("User-Agent", "Mozilla/4.76");
                    InputStream in = null;
                    try {
                        in = new BufferedInputStream(httpcon.getInputStream());
                    } catch (IOException ex) {
                        System.out.println("Warning: Problem downloading card: " + mappa.get(id) + "-" + id + " from " + scryset + " on ScryFall, i will retry 2 times more...");
                        try {
                            in = new BufferedInputStream(httpcon.getInputStream());
                        } catch (IOException ex2) {
                            System.out.println("Warning: Problem downloading card: " + mappa.get(id) + "-" + id + " from " + scryset + " on ScryFall, i will retry 1 time more...");
                            try {
                                in = new BufferedInputStream(httpcon.getInputStream());
                            } catch (IOException ex3) {
                                System.err.println("Error: Problem downloading card: " + mappa.get(id) + "-" + id + " from " + scryset + " on ScryFall, i will not retry anymore...");
                                break;
                            }
                        }
                    }
                    ByteArrayOutputStream out = new ByteArrayOutputStream();
                    byte[] buf = new byte[1024];
                    int n = 0;
                    while (-1 != (n = in.read(buf))) {
                        out.write(buf, 0, n);
                    }
                    out.close();
                    in.close();
                    byte[] response = out.toByteArray();
                    String cardimage = imgPath + File.separator + id + ".jpg";
                    String thumbcardimage = thumbPath + File.separator + id + ".jpg";
                    FileOutputStream fos = new FileOutputStream(cardimage);
                    fos.write(response);
                    fos.close();

                    Bitmap yourBitmap = BitmapFactory.decodeFile(cardimage);
                    Bitmap resized = Bitmap.createScaledBitmap(yourBitmap, ImgX, ImgY, true);
                    try {
                        FileOutputStream fout = new FileOutputStream(cardimage);
                        resized.compress(Bitmap.CompressFormat.JPEG, 100, fout);
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    Bitmap resizedThumb = Bitmap.createScaledBitmap(yourBitmap, ThumbX, ThumbY, true);
                    try {
                        FileOutputStream fout = new FileOutputStream(thumbcardimage);
                        resizedThumb.compress(Bitmap.CompressFormat.JPEG, 100, fout);
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    String text = "";
                    for (k = 0; k < divs.size(); k++)
                        if (divs.get(k).childNodes().size() > 0 && divs.get(k).childNode(0).toString().toLowerCase().contains("card text"))
                            break;
                    if (k < divs.size()) {
                        Element tex = divs.get(k + 1);
                        for (int z = 0; z < divs.get(k + 1).childNodes().size(); z++) {
                            for (int u = 0; u < divs.get(k + 1).childNode(z).childNodes().size(); u++) {
                                if (divs.get(k + 1).childNode(z).childNode(u).childNodes().size() > 1) {
                                    for (int w = 0; w < divs.get(k + 1).childNode(z).childNode(u).childNodes().size(); w++) {
                                        if (divs.get(k + 1).childNode(z).childNode(u).childNode(w).hasAttr("alt")) {
                                            String newtext = divs.get(k + 1).childNode(z).childNode(u).childNode(w).attributes().get("alt").trim();
                                            newtext = newtext.replace("Green", "{G}");
                                            newtext = newtext.replace("White", "{W}");
                                            newtext = newtext.replace("Black", "{B}");
                                            newtext = newtext.replace("Blue", "{U}");
                                            newtext = newtext.replace("Red", "{R}");
                                            newtext = newtext.replace("Tap", "{T}");
                                            text = text + newtext;
                                        } else
                                            text = text + " " + divs.get(k + 1).childNode(z).childNode(u).childNode(w).toString().replace("\r\n", "").trim() + " ";
                                        text = text.replace("} .", "}.");
                                        text = text.replace("} :", "}:");
                                        text = text.replace("} ,", "},");
                                    }
                                } else {
                                    if (divs.get(k + 1).childNode(z).childNode(u).hasAttr("alt")) {
                                        String newtext = divs.get(k + 1).childNode(z).childNode(u).attributes().get("alt").trim();
                                        newtext = newtext.replace("Green", "{G}");
                                        newtext = newtext.replace("White", "{W}");
                                        newtext = newtext.replace("Black", "{B}");
                                        newtext = newtext.replace("Blue", "{U}");
                                        newtext = newtext.replace("Red", "{R}");
                                        newtext = newtext.replace("Tap", "{T}");
                                        text = text + newtext;
                                    } else
                                        text = text + " " + divs.get(k + 1).childNode(z).childNode(u).toString().replace("\r\n", "").trim() + " ";
                                    text = text.replace("} .", "}.");
                                    text = text.replace("} :", "}:");
                                    text = text.replace("} ,", "},");
                                }
                                if (z > 0 && z < divs.get(k + 1).childNodes().size() - 1)
                                    text = text + " -- ";
                                text = text.replace("<i>", "");
                                text = text.replace("</i>", "");
                                text = text.replace("<b>", "");
                                text = text.replace("</b>", "");
                                text = text.replace(" -- (", " (");
                                text = text.replace("  ", " ");
                            }
                        }
                    }
                    if (hasToken(id) && ((text.trim().toLowerCase().contains("create") && text.trim().toLowerCase().contains("creature token")) || (text.trim().toLowerCase().contains("put") && text.trim().toLowerCase().contains("token")))) {
                        boolean tokenfound = false;
                        String arrays[] = text.trim().split(" ");
                        String nametoken = "";
                        String nametocheck = "";
                        String tokenstats = "";
                        for (int l = 1; l < arrays.length - 1; l++) {
                            if (arrays[l].equalsIgnoreCase("creature") && arrays[l + 1].toLowerCase().contains("token")) {
                                nametoken = arrays[l - 1];
                                if (l - 3 > 0)
                                    tokenstats = arrays[l - 3];
                                if (!tokenstats.contains("/")) {
                                    if (l - 4 > 0)
                                        tokenstats = arrays[l - 4];
                                }
                                if (!tokenstats.contains("/")) {
                                    if (l - 5 > 0)
                                        tokenstats = arrays[l - 5];
                                }
                                if (!tokenstats.contains("/")) {
                                    if (l - 6 > 0)
                                        tokenstats = arrays[l - 6];
                                }
                                if (!tokenstats.contains("/")) {
                                    if (l - 7 > 0)
                                        tokenstats = arrays[l - 7];
                                }
                                if (nametoken.equalsIgnoreCase("artifact")) {
                                    if (l - 2 > 0)
                                        nametoken = arrays[l - 2];
                                    if (l - 4 > 0)
                                        tokenstats = arrays[l - 4];
                                    if (!tokenstats.contains("/")) {
                                        if (l - 5 > 0)
                                            tokenstats = arrays[l - 5];
                                    }
                                    if (!tokenstats.contains("/")) {
                                        if (l - 6 > 0)
                                            tokenstats = arrays[l - 6];
                                    }
                                    if (!tokenstats.contains("/")) {
                                        if (l - 7 > 0)
                                            tokenstats = arrays[l - 7];
                                    }
                                    if (!tokenstats.contains("/")) {
                                        if (l - 8 > 0)
                                            tokenstats = arrays[l - 8];
                                    }
                                }
                                if (!tokenstats.contains("/"))
                                    tokenstats = "";
                                break;
                            } else if (arrays[l].equalsIgnoreCase("put") && arrays[l + 3].toLowerCase().contains("token")) {
                                nametoken = arrays[l + 2];
                                for (int j = 1; j < arrays.length - 1; j++) {
                                    if (arrays[j].contains("/"))
                                        tokenstats = arrays[j];
                                }
                                break;
                            }
                        }
                        String specialtokenurl = getSpecialTokenUrl(id + "t");
                        Elements imgstoken;
                        if (!specialtokenurl.isEmpty()) {
                            try {
                                doc = Jsoup.connect(imageurl + scryset.toLowerCase()).get();
                            } catch (Exception ex) {
                                System.err.println("Error: Problem occurring while searching for token: " + nametoken + "-" + id + "t, i will not download it...");
                                break;
                            }
                            if (doc == null)
                                break;
                            imgstoken = doc.select("body img");
                            if (imgstoken == null)
                                break;
                            tokenfound = true;
                        } else {
                            if (nametoken.isEmpty() || tokenstats.isEmpty()) {
                                tokenfound = false;
                                if (nametoken.isEmpty())
                                    nametoken = "Unknown";
                                nametocheck = mappa.get(id);
                                doc = Jsoup.connect(imageurl + scryset.toLowerCase()).get();
                            } else {
                                try {
                                    doc = findTokenPage(imageurl, nametoken, scryset, availableSets, tokenstats, parent);
                                    tokenfound = true;
                                    nametocheck = nametoken;
                                } catch (Exception e) {
                                    tokenfound = false;
                                    nametocheck = mappa.get(id);
                                    doc = Jsoup.connect(imageurl + scryset.toLowerCase()).get();
                                }
                            }
                            if (doc == null)
                                break;
                            imgstoken = doc.select("body img");
                            if (imgstoken == null)
                                break;
                        }
                        for (int p = 0; p < imgstoken.size() && parent.downloadInProgress; p++) {
                            while (parent.paused && parent.downloadInProgress) {
                                try {
                                    Thread.sleep(1000);
                                } catch (InterruptedException e) {
                                }
                            }
                            if (!parent.downloadInProgress)
                                break;

                            String titletoken = imgstoken.get(p).attributes().get("alt");
                            if (titletoken.isEmpty())
                                titletoken = imgstoken.get(p).attributes().get("title");
                            if (titletoken.toLowerCase().contains(nametocheck.toLowerCase())) {
                                String CardImageToken = imgstoken.get(p).attributes().get("src");
                                if (CardImageToken.isEmpty())
                                    CardImageToken = imgstoken.get(p).attributes().get("data-src");
                                URL urltoken = new URL(CardImageToken);
                                if (!specialtokenurl.isEmpty())
                                    urltoken = new URL(specialtokenurl);
                                HttpURLConnection httpcontoken = (HttpURLConnection) urltoken.openConnection();
                                if (httpcontoken == null) {
                                    System.err.println("Error: Problem downloading token: " + nametoken + "-" + id + "t, i will not download it...");
                                    break;
                                }
                                httpcontoken.addRequestProperty("User-Agent", "Mozilla/4.76");
                                InputStream intoken = null;
                                try {
                                    intoken = new BufferedInputStream(httpcontoken.getInputStream());
                                } catch (IOException ex) {
                                    System.out.println("Warning: Problem downloading token: " + nametoken + "-" + id + "t, i will retry 2 times more...");
                                    try {
                                        intoken = new BufferedInputStream(httpcontoken.getInputStream());
                                    } catch (IOException ex2) {
                                        System.out.println("Warning: Problem downloading token: " + nametoken + "-" + id + "t, i will retry 1 time more...");
                                        try {
                                            intoken = new BufferedInputStream(httpcontoken.getInputStream());
                                        } catch (IOException ex3) {
                                            System.err.println("Error: Problem downloading token: " + nametoken + "-" + id + "t, i will not retry anymore...");
                                            break;
                                        }
                                    }
                                }
                                ByteArrayOutputStream outtoken = new ByteArrayOutputStream();
                                byte[] buftoken = new byte[1024];
                                int ntoken = 0;
                                while (-1 != (ntoken = intoken.read(buftoken))) {
                                    outtoken.write(buftoken, 0, ntoken);
                                }
                                outtoken.close();
                                intoken.close();
                                byte[] responsetoken = outtoken.toByteArray();
                                String tokenimage = imgPath + File.separator + id + "t.jpg";
                                String tokenthumbimage = thumbPath + File.separator + id + "t.jpg";
                                if (!tokenfound && !id.equals("464007")) {
                                    System.err.println("Error: Problem downloading token: " + nametoken + " (" + id + "t) i will use the same image of its source card");
                                }
                                FileOutputStream fos2 = new FileOutputStream(tokenimage);
                                fos2.write(responsetoken);
                                fos2.close();

                                Bitmap yourBitmapToken = BitmapFactory.decodeFile(tokenimage);
                                Bitmap resizedToken = Bitmap.createScaledBitmap(yourBitmapToken, ImgX, ImgY, true);
                                try {
                                    FileOutputStream fout = new FileOutputStream(tokenimage);
                                    resizedToken.compress(Bitmap.CompressFormat.JPEG, 100, fout);
                                } catch (IOException e) {
                                    e.printStackTrace();
                                }
                                Bitmap resizedThumbToken = Bitmap.createScaledBitmap(yourBitmapToken, ThumbX, ThumbY, true);
                                try {
                                    FileOutputStream fout = new FileOutputStream(tokenthumbimage);
                                    resizedThumbToken.compress(Bitmap.CompressFormat.JPEG, 100, fout);
                                } catch (IOException e) {
                                    e.printStackTrace();
                                }

                                break;
                            }
                        }
                    }
                    break;
                }
            }
        }

        while (parent.paused && parent.downloadInProgress) {
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
            }
        }

        if (parent.downloadInProgress) {
            try {
                try {
                    File oldzip = new File(destinationPath + File.separator + set + File.separator + set + ".zip");
                    oldzip.delete();
                } catch (Exception e) {
                }
                ZipParameters zipParameters = new ZipParameters();
                zipParameters.setCompressionMethod(CompressionMethod.STORE);
                File folder = new File(destinationPath + set + File.separator);
                File[] listOfFile = folder.listFiles();
                net.lingala.zip4j.ZipFile zipped = new net.lingala.zip4j.ZipFile(destinationPath + File.separator + set + File.separator + set + ".zip");
                for (int i = 0; i < listOfFile.length; i++) {
                    if (listOfFile[i].isDirectory()) {
                        zipped.addFolder(listOfFile[i], zipParameters);
                    } else {
                        zipped.addFile(listOfFile[i], zipParameters);
                    }
                }
                File destFolder = new File(destinationPath + set + File.separator);
                listOfFiles = destFolder.listFiles();
                for (int u = 0; u < listOfFiles.length; u++) {
                    if (!listOfFiles[u].getName().contains(".zip")) {
                        if (listOfFiles[u].isDirectory()) {
                            File[] listOfSubFiles = listOfFiles[u].listFiles();
                            for (int j = 0; j < listOfSubFiles.length; j++)
                                listOfSubFiles[j].delete();
                        }
                        listOfFiles[u].delete();
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return res;
    }
}
